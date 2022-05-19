# Use this instead of 'add_library(target_name [INTERFACE|STATIC] [sources ...])'
# - Automatically create INTERFACE target when only headers are in sources
macro(engine_library TARGET)
	set(_target ${ARGV0})
	set(_sources ${ARGN})
	set(_headers ${ARGN})
	list(FILTER _headers INCLUDE REGEX ".*.hpp$")

	list(LENGTH _sources _sources_count)
	list(LENGTH _headers _headers_count)

	set(_library_type STATIC)
	set(_export_sources PUBLIC)
	set(_export PUBLIC)

	if (_sources_count EQUAL _headers_count)
		list(TRANSFORM _sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
		set(_library_type INTERFACE)
		set(_export_sources INTERFACE)
		set(_export INTERFACE)
	endif()

	add_library(${_target} ${_library_type})
	
	target_sources(${_target} ${_export_sources} ${_sources})

	target_link_libraries(${_target} ${_export})

  	target_include_directories(${_target} ${_export_sources} ${CMAKE_CURRENT_SOURCE_DIR})

	target_compile_options(${_target} ${_export})
	target_compile_definitions(${_target} ${_export})
endmacro()

# Use this instead of 'target_link_libraries(target_name [INTERFACE|STATIC] [libs ...])'
# - Default to PUBLIC keyword
# - Automatically use INTERFACE keyword when target is INTERFACE
macro(engine_link_libraries TARGET)
	set(_target ${ARGV0})
	set(_libraries ${ARGN})

	set(_public_libs)
	set(_private_libs)
	set(_current PUBLIC)

	foreach(_library ${_libraries})
		if (${_library} STREQUAL "PUBLIC")
			set(_current PUBLIC)
		elseif(${_library} STREQUAL "PRIVATE")
			set(_current PRIVATE)
		else()
			if (${_current} STREQUAL "PUBLIC")
				list(APPEND _public_libs ${_library})
			elseif (${_current} STREQUAL "PRIVATE")
				list(APPEND _private_libs ${_library})
			endif()
		endif()
	endforeach()

	get_target_property(_type ${_target} TYPE)

	if (${_type} STREQUAL "INTERFACE_LIBRARY")
		target_link_libraries(${_target} INTERFACE ${_public_libs})
		target_link_libraries(${_target} INTERFACE ${_private_libs})
	else()
		target_link_libraries(${_target} PUBLIC ${_public_libs})
		target_link_libraries(${_target} PRIVATE ${_private_libs})
	endif()
endmacro()