#
# Copyright (c) 2008-2021 the Urho3D project.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# Define source files
set (SOURCE_FILES
	Include/FJS/detail/Delegate.h
	Include/FJS/detail/detail.h
	Include/FJS/detail/function_checks.h
	Include/FJS/detail/function_traits.h
	Include/FJS/detail/mpmc_queue.h
	Include/FJS/Common.h
	Include/FJS/Counter.h
	Include/FJS/Fiber.h
	Include/FJS/Job.h
	Include/FJS/List.h
	Include/FJS/Manager.h
	Include/FJS/Queue.h
	Include/FJS/Thread.h
	Include/FJS/TLS.h
	
	Source/Counter.cpp
	Source/Fiber.cpp
	Source/Job.cpp
	Source/List.cpp
	Source/Manager.cpp
	Source/ManagerCallbacks.cpp
	Source/ManagerJobs.cpp
	Source/ManagerThreads.cpp
	Source/Queue.cpp
	Source/Thread.cpp
)

add_library (FiberJobSystem STATIC ${SOURCE_FILES})

if (NOT MINI_URHO)
    if (NOT URHO3D_MERGE_STATIC_LIBS)
        install(TARGETS FiberJobSystem EXPORT Urho3D ARCHIVE DESTINATION ${DEST_ARCHIVE_DIR_CONFIG})
    endif ()
endif ()
