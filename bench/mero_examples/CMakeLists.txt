
function(add_mero_test test_name src)

	add_executable(mero_${test_name}.out ${src})

	target_include_directories(mero_${test_name}.out PRIVATE ${CMAKE_SOURCE_DIR}/src/public-api)
	set_target_properties(mero_${test_name}.out PROPERTIES COMPILE_FLAGS "${MERO_COMPILE_FLAGS} -DHAVE_MERO")
	set_target_properties(mero_${test_name}.out PROPERTIES LINK_FLAGS "${MERO_LINK_FLAGS}")
	target_link_libraries(mero_${test_name}.out ummap-io)

endfunction()

function(add_motr_test test_name src)

	add_executable(motr_${test_name}.out ${src})

	target_include_directories(motr_${test_name}.out PRIVATE ${CMAKE_SOURCE_DIR}/src/public-api)
	set_target_properties(motr_${test_name}.out PROPERTIES COMPILE_FLAGS "${MOTR_COMPILE_FLAGS} -DHAVE_MOTR")
	set_target_properties(motr_${test_name}.out PROPERTIES LINK_FLAGS "${MOTR_LINK_FLAGS}")
	target_link_libraries(motr_${test_name}.out ummap-io)

endfunction()


if (MERO_FOUND)
	add_mero_test(hello_world ./hello_world/mero_example.c)
	add_mero_test(big_ios ./big_ios/big_ios.c)
	add_custom_target(mero_examples DEPENDS mero_example.out big_ios.out)
endif()

if (MOTR_FOUND)
	add_motr_test(hello_world ./hello_world/motr_example.c)
	add_motr_test(big_ios ./big_ios/big_ios.c)
	add_custom_target(mero_examples DEPENDS motr_example.out big_ios.out)
endif()
