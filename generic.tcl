catch {eval exec "uname -m"} machine_name
catch {eval exec "uname -s"} kernel_name
catch {eval exec "vivado_hls -r"} vivado_hls_home

if {$machine_name == "x86_64" && $kernel_name == "Linux"} {
    set gcc_lib $vivado_hls_home/Linux_x86_64/tools/gcc/lib64
} elseif {$machine_name == "i686" && $kernel_name == "Linux"} {
    set gcc_lib $vivado_hls_home/Linux_x86/tools/gcc/lib
} else {
    set gcc_lib $vivado_hls_home/msys/lib/gcc/mingw32/4.6.2
}
set ::env(LD_LIBRARY_PATH) $gcc_lib:$::env(LD_LIBRARY_PATH)

# Create the design project
open_project prj -reset

# Define the top level function for hardware synthesis
set_top image_filter

# Select the files for hardware synthesis
add_files top.cpp


# Select the files required for the testbench
add_files -tb test.cpp
add_files -tb opencv_top.cpp
add_files -tb test_1080p.bmp

# Set the name of the solution for this design
open_solution "solution1"

# Select the FPGA 
set_part xc7z020clg484-1
create_clock -period "150MHz"

# Vivado HLS commands for C simulation
csim_design

# Vivado HLS commands for C to RTL synthesis
csynth_design

# Vivado HLS commands for RTL simulation
#cosim_design

# Vivado HLS commands for RTL implementation
export_design -format pcore -version 1.05.a

exit
