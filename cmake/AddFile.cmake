set(CMAKE_CURRENT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE STRING "" FORCE)

# Convert file into a binary blob and add it to the list of SOURCES.
macro(add_file FILE_PATH SOURCES)
	set(XXD xxd)
	set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir/${XXD}") 
	get_filename_component(FILE_NAME ${FILE_PATH} NAME)

	# Translate OpenCL source into comma-separated byte codes.
	set(FILE_HEX "${OUTPUT_DIRECTORY}/${FILE_NAME}.hex")
	add_custom_command(
		OUTPUT ${FILE_HEX}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}"
		COMMAND ${XXD} -i < ${FILE_PATH} > ${FILE_HEX}
		COMMENT "Generating hex representation of ${FILE_PATH} file content"
		DEPENDS ${FILE_PATH} ${XXD})
	set_source_files_properties("${FILE_HEX}" PROPERTIES GENERATED TRUE) 
	set(EMBED_FILE_PATH "${OUTPUT_DIRECTORY}/${FILE_NAME}.cpp")
	add_custom_command(
		OUTPUT ${EMBED_FILE_PATH}
		COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}"
		COMMAND ${CMAKE_COMMAND} -DCMAKE_CURRENT_INCLUDE_DIR=${CMAKE_CURRENT_INCLUDE_DIR} -DFILE_HEX=${FILE_HEX} -DFILE_NAME=${FILE_NAME} -DFILE_PATH=${EMBED_FILE_PATH} -P ${CMAKE_CURRENT_INCLUDE_DIR}/../cmake/EmbedFile.cmake
		COMMENT "Embedding file ${FILE_PATH}"
		DEPENDS ${FILE_HEX} "${CMAKE_CURRENT_INCLUDE_DIR}/www.in")
	set_source_files_properties("${EMBED_FILE_PATH}" PROPERTIES GENERATED TRUE) 
	# Submit the resulting source file for compilation
	list(APPEND ${SOURCES} ${EMBED_FILE_PATH})
	message(STATUS "File ${FILE_PATH} shall be added to ${SOURCES}")
endmacro()

