// Copyright (c) Microsoft
// All rights reserved
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED,
// INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache Version 2.0 License for specific language governing permissions and limitations under the License.
/// <tags>P1</tags>
/// <summary>Attempt to use View_As with a longer size and shorter size than the original</summary>

#include <amptest.h>
#include <amptest_main.h>

using namespace Concurrency;
using namespace Concurrency::Test;

runall_result test_main()
{
	runall_result result;
    array<int, 1> arr(10);
    try
    {
        // this should throw
        array_view<int, 2> r = arr.view_as(extent<2>(3, 4));
        result &= runall_fail;
    }
    catch (runtime_exception &re)
    {
        Log() << re.what() << std::endl;
		result &= runall_pass;
    }
	
	try
    {
        // this should not throw
        array_view<int, 2> r = arr.view_as(extent<2>(2, 4));
        result &= runall_pass;
    }
    catch (runtime_exception &re)
    {
        Log() << re.what() << std::endl;
		result &= runall_fail;
    }
	return result;
}

