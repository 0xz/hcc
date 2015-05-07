// Copyright (c) Microsoft
// All rights reserved
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache Version 2.0 License for specific language governing permissions and limitations under the License.
//<tags>P0,M3</tags>
/// <summary>Control Flow test: In if statement, “else” path shall be tested.
/// Call group_barrier to do some computation by using shared memory in “else” path.</summary>

#include <iostream>
#include <amptest.h>
#include <amptest_main.h>

using namespace std;
using namespace Concurrency;
using namespace Concurrency::Test;

const int XSize      = 8;
const int YSize      = 8;
const int Size       = XSize * YSize;

const int XGroupSize = 4;
const int YGroupSize = 4;

const int NumXGroups = XSize / XGroupSize;           // Make sure that Size is divisible by GroupSize
const int NumYGroups = YSize / YGroupSize;           // Make sure that Size is divisible by GroupSize
const int NumGroups  =  NumXGroups * NumYGroups;     // Make sure that Size is divisible by GroupSize

void CalculateGroupSum(int* A, int* B)
{
    int g = 0;
    for(int y = 0; y < YSize; y += YGroupSize)
    {
        for(int x = 0; x < XSize; x += XGroupSize)
        {
            // x,y is now the origin of the next group
            // If group 0, then don't calculaue sum
            if(g != 0)
            {
                B[g] = 0;
                // calculate sum
                for(int gy = y; gy < (y + YGroupSize); gy++)
                {
                    for(int gx = x; gx < (x + XGroupSize); gx++)
                    {
                        int flatLocalIndex = gy * XSize + gx;
                        B[g] += A[flatLocalIndex];
                    }
                }
            }
            else
            {
                B[g] = 100;
            }

            g++;
        }
    }
}

//Calculate sum of all elements in a group - GPU version
void CalculateGroupSum(tiled_index<YGroupSize, XGroupSize> ti, int flatLocalIndex, Concurrency::array<int, 2>& fA, Concurrency::array<int, 2>& fB) __GPU_ONLY
{
    // use shared memory
    tile_static int shared[XGroupSize * YGroupSize];
    shared[flatLocalIndex] = fA[ti.global];
    ti.barrier.wait();

    if(flatLocalIndex == 0)
    {
        int sum = 0;
        for(int i = 0; i < XGroupSize * YGroupSize; i++)
        {
            sum += shared[i];
        }

        fB[ti.tile] = sum;
    }
}

void kernel(tiled_index<YGroupSize, XGroupSize> ti, Concurrency::array<int, 2>& fA, Concurrency::array<int, 2>& fB, int x) __GPU_ONLY
{
    int flatLocalIndex = ti.local[0] * XGroupSize + ti.local[1];
    int groupIndex = ti.tile[0] * XGroupSize + ti.tile[1];

    // group 0 initialized to fixed value; other thread groups calculate group sum
    if(groupIndex == 0)
    {
        if(flatLocalIndex == 0) fB[ti.tile] = 100;
    }
    else
    {
        CalculateGroupSum(ti, flatLocalIndex, fA, fB);
    }
}

runall_result test_main()
{

    bool passed = true;

    vector<int> A(Size); // data
    vector<int> B(NumGroups);   // holds the grouped sum of data

    vector<int> refB(NumGroups); // Expected value ; sum of elements in each group

    //Init A
    Fill<int>(A.data(), Size, 0, 100);

    //Init expected values
    CalculateGroupSum(A.data(), refB.data());

    accelerator_view av =  require_device(Device::ALL_DEVICES).get_default_view();

    Concurrency::extent<2> extentA(XSize, YSize), extentB(NumYGroups, NumXGroups);
    Concurrency::array<int, 2> fA(extentA, A.begin(), A.end(), av), fB(extentB, av);

    //parallel_for_each where conditions are met
    parallel_for_each(fA.get_extent().tile<YGroupSize, XGroupSize>(), [&](tiled_index<YGroupSize, XGroupSize> ti) __GPU_ONLY{
        int x = 123;
        kernel(ti, fA, fB, x);
    });

    B = fB;
    if(!Verify<int>(B.data(), refB.data(), NumGroups))
    {
        passed = false;
        cout << "Test: failed" << endl;
        return runall_fail;
    }
    else
    {
        cout << "Test: passed" << endl;
        return runall_pass;
    }

    return runall_pass;
}
