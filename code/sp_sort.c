#include "opengl.h"
#include <stdlib.h>
enum SortingTypes
{
    SortType_BubbleSort,
    SortType_InsertionSort,
    SortType_MergeSort,
    SortType_RadixSort,
};

typedef struct sort_step
{
    int From;
    int To;
}sort_step;

typedef struct sorting_data
{
    int Type;
    int ElementCount;
    int *Elements;
    int *SortedElements;
    
	int AllocatedSteps;
    int SortStepCount;
    int CurrentStep;
    sort_step *SortSteps;
}sorting_data;

static sorting_data *SortingAlgos;
static int SortingAlgoCount;

static void
AllocStep(sorting_data *Data){
    if(Data->SortStepCount >= Data->AllocatedSteps)
    {
        Data->AllocatedSteps *= 2;
        Data->SortSteps= PlatformReallocate(Data->SortSteps, sizeof(sort_step)*Data->AllocatedSteps);
    }
}

static void
FillRandom(sorting_data *Data, u32 Max)
{
    if(!Data->Elements)
    {
        Data->Elements = PlatformAllocate(sizeof(int) * Data->ElementCount);
    }
    for(int i = 0; i < Data->ElementCount; ++i)
    {
        Data->Elements[i] = rand() % Max;
    }
    if(!Data->SortedElements)
    {
        Data->SortedElements = PlatformAllocate(sizeof(int) * Data->ElementCount);
        MemoryCopy(Data->SortedElements, Data->Elements, sizeof(int) * Data->ElementCount);
    }
}

static sort_step
Swap(int *Elements, int a, int b)
{
    sort_step SortStep = {.From = a, .To = b};
    int temp = Elements[a];
    Elements[a] = Elements[b];
    Elements[b] = temp;
    return(SortStep);
}

static void
BubbleSort(sorting_data *Data)
{
    int n = Data->ElementCount;
	Data->SortSteps = PlatformAllocate(sizeof(sort_step));
    while(n != 0)
    {
        int newn = 0;
        for(int i = 1; i <= n - 1; ++i)
        {
            if(Data->SortedElements[i-1] > Data->SortedElements[i])
            {
				AllocStep(Data);
                Data->SortSteps[Data->SortStepCount++] = Swap(Data->SortedElements, i-1, i);
                newn = i;
            }
        }
        n = newn;
    }
    PlatformDeallocate(Data->SortedElements);
}

static void
InsertionSort(sorting_data *Data){
    int n = Data->ElementCount, j, element;
	Data->SortSteps = PlatformAllocate(sizeof(sort_step));
	
    for(int i = 1; i <= n - 1; ++i)
    {
		element = Data->SortedElements[i];
		j = i;
		while( j > 0 && Data->SortedElements[j-1] > element)
		{
			AllocStep(Data);
			Data->SortSteps[Data->SortStepCount++] = Swap(Data->SortedElements, j-1, j);
			j--;
		}

    }
	PlatformDeallocate(Data->SortedElements);
}



static void
AdvanceStep(sorting_data *Data, int Amount)
{
    if(Amount > 0)
    {
        for(int i = 0; i < Amount; i++)
        {
            if(Data->CurrentStep + i < 0 || Data->CurrentStep + i > Data->SortStepCount) break;
            Swap(Data->Elements, Data->SortSteps[Data->CurrentStep + i].From, Data->SortSteps[Data->CurrentStep + i].To);
        }
    }
    else if(Amount < 0)
    {
        for(int i = -1; i >= Amount; i--)
        {
            if(Data->CurrentStep + i < 0 || Data->CurrentStep + i > Data->SortStepCount) break;
            Swap(Data->Elements, Data->SortSteps[Data->CurrentStep + i].From, Data->SortSteps[Data->CurrentStep + i].To);
        }
    }
    Data->CurrentStep += Amount;
    if(Data->CurrentStep < 0)
    {
        Data->CurrentStep = -1;
    }
    else if(Data->CurrentStep > Data->SortStepCount - 1)
    {
        Data->CurrentStep = Data->SortStepCount;
    }
}

UPDATE_AND_RENDER(UpdateAndRenderFunc)
{	
    static b32 FastMode = false;
    static b32 Initialized = false;
	static u32 curr = SortType_BubbleSort;
    if(!Initialized)
    {
        SortingAlgoCount = 2;
        SortingAlgos = PlatformAllocate(sizeof(sorting_data) * SortingAlgoCount);
		SortingAlgos[0].Type = SortType_BubbleSort;
		SortingAlgos[1].Type = SortType_InsertionSort;
		for(u32 i = 0; i<SortingAlgoCount; i++){
			MemorySet(&SortingAlgos[i], 0, sizeof(sorting_data));
			SortingAlgos[i].Type = SortType_BubbleSort;
			SortingAlgos[i].CurrentStep = -1;
			SortingAlgos[i].AllocatedSteps = 1;
			SortingAlgos[i].ElementCount = 40;
			FillRandom(&SortingAlgos[i], SortingAlgos[i].ElementCount);
		}
		
        BubbleSort(&SortingAlgos[0]);
		InsertionSort(&SortingAlgos[1]);
        Initialized = true;
    }
    if(!FastMode)
    {
        if(KeyDown(Input_Left, Input))
        {
            AdvanceStep(&SortingAlgos[curr], -1);
        }
        if(KeyDown(Input_Right, Input))
        {
            AdvanceStep(&SortingAlgos[curr], 1);
        }
        if(KeyDown('R', Input))
        {
            AdvanceStep(&SortingAlgos[curr], -SortingAlgos[curr].CurrentStep);
        }
    }
    
    if(KeyDown('F', Input))
    {
        FastMode = !FastMode;
    }
	if(KeyDown('1', Input))
    {
        curr = SortType_BubbleSort;
    }
	else if(KeyDown('2', Input))
    {
        curr = SortType_InsertionSort;
    }
    if(FastMode)
    {
        AdvanceStep(&SortingAlgos[curr], 1);
        if(SortingAlgos[curr].CurrentStep >= SortingAlgos[curr].SortStepCount)
        {
            FastMode = false;
        }
    }
    
    v4 Colors[2] =
    {
        0,1,0,1,
        0,0.5f,0,1
    };
    v4 HighlightColors[2] =
    {
        0,1,1,1,
        0,0.5f,0.5f,1
    };
    
    float BarWidth = Width / SortingAlgos[curr].ElementCount;
    float BarHeightMultiply = 20;
    
    for(u32 i = 0; i < SortingAlgos[curr].ElementCount; ++i)
    {
        v4 Color = Colors[i%2];
        DrawQuad(i * BarWidth + 1, 0, BarWidth - 1, SortingAlgos[curr].Elements[i] * BarHeightMultiply, Color);
    }
    if(SortingAlgos[curr].CurrentStep >= 0 && SortingAlgos[curr].CurrentStep < SortingAlgos[curr].SortStepCount)
    {
        int i = SortingAlgos[curr].SortSteps[SortingAlgos[curr].CurrentStep].From;
        DrawQuad(i * BarWidth + 1, 0, BarWidth - 1, SortingAlgos[curr].Elements[i] * BarHeightMultiply, HighlightColors[0]);
        i = SortingAlgos[curr].SortSteps[SortingAlgos[curr].CurrentStep].To;
        DrawQuad(i * BarWidth + 1, 0, BarWidth - 1, SortingAlgos[curr].Elements[i] * BarHeightMultiply, HighlightColors[1]);
    }
}