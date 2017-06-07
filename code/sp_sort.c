#include "opengl.h"
#include <stdlib.h>
enum SortingTypes
{
    SortType_BubbleSort,
    SortType_MergeSort,
    SortType_InsertionSort,
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
    
    int SortStepCount;
    int CurrentStep;
    sort_step *SortSteps;
}sorting_data;

static sorting_data *SortingAlgos;
static int SortingAlgoCount;

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
    int SortStepAllocCount = 1;
    int n = Data->ElementCount;
    if(!Data->SortSteps)
    {
        Data->SortSteps = PlatformAllocate(sizeof(sort_step) * SortStepAllocCount);
    }
    while(n != 0)
    {
        int newn = 0;
        for(int i = 1; i <= n - 1; ++i)
        {
            if(Data->SortedElements[i-1] > Data->SortedElements[i])
            {
                if(Data->SortStepCount > SortStepAllocCount - 1)
                {
                    SortStepAllocCount *= 2;
                    Data->SortSteps= PlatformReallocate(Data->SortSteps, sizeof(sort_step) * SortStepAllocCount);
                }
                Data->SortSteps[Data->SortStepCount++] = Swap(Data->SortedElements, i-1, i);
                newn = i;
            }
        }
        n = newn;
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
    if(!Initialized)
    {
        SortingAlgoCount = 1;
        SortingAlgos = PlatformAllocate(sizeof(sorting_data) * SortingAlgoCount);
        MemorySet(&SortingAlgos[0], 0, sizeof(sorting_data));
        SortingAlgos[0].Type = SortType_BubbleSort;
        SortingAlgos[0].CurrentStep = -1;
        SortingAlgos[0].ElementCount = 10;
        FillRandom(&SortingAlgos[0], SortingAlgos[0].ElementCount);
        BubbleSort(&SortingAlgos[0]);
        Initialized = true;
    }
    if(!FastMode)
    {
        if(KeyDown(Input_Left, Input))
        {
            AdvanceStep(&SortingAlgos[0], -1);
        }
        if(KeyDown(Input_Right, Input))
        {
            AdvanceStep(&SortingAlgos[0], 1);
        }
        if(KeyDown('R', Input))
        {
            AdvanceStep(&SortingAlgos[0], -SortingAlgos[0].CurrentStep);
        }
    }
    
    if(KeyDown('F', Input))
    {
        FastMode = !FastMode;
    }
    if(FastMode)
    {
        AdvanceStep(&SortingAlgos[0], 1);
        if(SortingAlgos[0].CurrentStep >= SortingAlgos[0].SortStepCount)
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
    
    float BarWidth = Width / SortingAlgos[0].ElementCount;
    float BarHeightMultiply = 20;
    
    for(u32 i = 0; i < SortingAlgos[0].ElementCount; ++i)
    {
        v4 Color = Colors[i%2];
        DrawQuad(i * BarWidth + 1, 0, BarWidth - 1, SortingAlgos[0].Elements[i] * BarHeightMultiply, Color);
    }
    if(SortingAlgos[0].CurrentStep >= 0 && SortingAlgos[0].CurrentStep < SortingAlgos[0].SortStepCount)
    {
        int i = SortingAlgos[0].SortSteps[SortingAlgos[0].CurrentStep].From;
        DrawQuad(i * BarWidth + 1, 0, BarWidth - 1, SortingAlgos[0].Elements[i] * BarHeightMultiply, HighlightColors[0]);
        i = SortingAlgos[0].SortSteps[SortingAlgos[0].CurrentStep].To;
        DrawQuad(i * BarWidth + 1, 0, BarWidth - 1, SortingAlgos[0].Elements[i] * BarHeightMultiply, HighlightColors[1]);
    }
}