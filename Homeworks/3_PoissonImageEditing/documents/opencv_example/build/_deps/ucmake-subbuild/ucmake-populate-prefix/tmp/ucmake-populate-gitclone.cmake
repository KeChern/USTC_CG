
if(NOT "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp/ucmake-populate-gitinfo.txt" IS_NEWER_THAN "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp/ucmake-populate-gitclone-lastrun.txt")
  message(STATUS "Avoiding repeated git clone, stamp file is up to date: 'D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp/ucmake-populate-gitclone-lastrun.txt'")
  return()
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E remove_directory "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: 'D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "D:/Program Files/Git/cmd/git.exe"  clone --no-checkout "https://github.com/Ubpa/UCMake" "ucmake-src"
    WORKING_DIRECTORY "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps"
    RESULT_VARIABLE error_code
    )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(STATUS "Had to git clone more than once:
          ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/Ubpa/UCMake'")
endif()

execute_process(
  COMMAND "D:/Program Files/Git/cmd/git.exe"  checkout 0.6.0 --
  WORKING_DIRECTORY "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-src"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: '0.6.0'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "D:/Program Files/Git/cmd/git.exe"  submodule update --recursive --init 
    WORKING_DIRECTORY "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-src"
    RESULT_VARIABLE error_code
    )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: 'D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy
    "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp/ucmake-populate-gitinfo.txt"
    "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp/ucmake-populate-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  )
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: 'D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp/ucmake-populate-gitclone-lastrun.txt'")
endif()

