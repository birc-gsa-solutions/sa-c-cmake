cmake_minimum_required (VERSION 3.21)
project (sa)
enable_testing()

add_subdirectory(src)

add_executable(sa src/sa.c src/fasta.c src/fastq.c src/sam.c)
target_link_libraries(sa cstr)
