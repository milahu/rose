add_executable(statements statements.C )
target_link_libraries(statements ROSE_DLL EDG ${link_with_libraries} )
install(TARGETS statements DESTINATION bin)

if(NOT with-ROSE_LONG_MAKE_CHECK_RULE)
  return()
endif()

set(TESTCODES
  test_CplusplusMacro_Cpp.C test2001_01.C test2001_02.C test2001_03.C
  test2001_04.C test2001_05.C test2001_06.C test2001_07.C test2001_08.C
  test2001_09.C test2001_10.C test2001_11.C test2001_12.C test2001_13.C
  test2001_14.C test2001_15.C test2001_16.C test2001_17.C test2001_18.C
  test2001_19.C test2001_20.C test2001_21.C test2001_22.C test2001_23.C
  test2001_24.C test2001_25.C test2001_26.C test2001_27.C test2001_28.C
  test2001_29.C test2002_01.C test2003_01.C test2003_02.C test2003_03.C
  test2003_04.C test2003_05.C test2003_06.C test2003_08.C test2003_09.C
  test2003_10.C test2003_11.C test2003_12.C test2003_13.C test2003_14.C
  test2003_15.C test2003_16.C test2003_17.C test2003_18.C test2003_19.C
  test2003_20.C test2003_21.C test2003_23.C test2003_24.C test2003_25.C
  test2003_26.C test2003_27.C test2003_28.C test2003_29.C test2003_30.C
  test2003_31.C test2003_32.C test2003_35.C test2003_36.C test2003_37.C
  test2003_38.C test2004_01.C test2004_02.C test2004_03.C test2004_04.C
  test2004_05.C test2004_06.C test2004_07.C test2004_08.C test2004_09.C
  test2004_10.C test2004_11.C test2004_12.C test2004_13.C test2004_14.C
  test2004_15.C test2004_16.C test2004_17.C test2004_18.C test2004_19.C
  test2004_23.C test2004_24.C test2004_26.C test2004_27.C test2004_28.C
  test2004_29.C test2004_30.C test2004_31.C test2004_32.C test2004_33.C
  test2004_34.C test2004_35.C test2004_36.C test2004_37.C test2004_38.C
  test2004_39.C test2004_40.C test2004_42.C test2004_43.C test2004_44.C
  test2004_45.C test2004_47.C test2004_48.C test2004_49.C test2004_50.C
  test2004_51.C test2004_52.C test2004_53.C test2004_54.C test2004_55.C
  test2004_56.C test2004_58.C test2004_59.C test2004_60.C test2004_61.C
  test2004_62.C test2004_63.C test2004_64.C test2004_65.C test2004_67.C
  test2004_68.C test2004_69.C test2004_70.C test2004_71.C test2004_72.C
  test2004_73.C test2004_74.C test2004_75.C test2004_76.C test2004_77.C
  test2004_78.C test2004_79.C test2004_80.C test2004_81.C test2004_82.C
  test2004_84.C test2004_85.C test2004_86.C test2004_87.C test2004_88.C
  test2004_89.C test2004_90.C test2004_91.C test2004_93.C test2004_94.C
  test2004_95.C test2004_96.C test2004_97.C test2004_98.C test2004_99.C
  test2004_100.C test2004_101.C test2004_102.C test2004_103.C test2004_104.C
  test2004_105.C test2004_106.C test2004_107.C test2004_108.C test2004_109.C
  test2004_110.C test2004_111.C test2004_112.C test2004_113.C test2004_114.C
  test2004_115.C test2004_116.C test2004_117.C test2004_118.C test2004_119.C
  test2004_120.C test2004_121.C test2004_122.C test2004_123.C test2004_124.C
  test2004_125.C test2004_127.C test2004_128.C test2004_129.C test2004_130.C
  test2004_131.C test2004_132.C test2004_133.C test2004_134.C test2004_135.C
  test2004_136.C test2004_137.C test2004_138.C test2004_139.C test2004_140.C
  test2004_141.C test2004_142.C test2004_143.C test2004_144.C test2004_145.C
  test2004_146.C test2004_147.C test2004_148.C test2004_149.C test2004_150.C
  test2004_151.C test2004_152.C test2004_153.C test2004_154.C test2004_155.C
  test2004_156.C test2004_157.C test2004_158.C test2004_159.C test2004_160.C
  test2004_161.C test2004_162.C test2004_163.C test2004_164.C test2005_01.C
  test2005_02.C test2005_03.C test2005_05.C test2005_07.C test2005_08.C
  test2005_09.C test2005_10.C test2005_11.C test2005_12.C test2005_13.C
  test2005_14.C test2005_15.C test2005_16.C test2005_17.C test2005_19.C
  test2005_20.C test2005_21.C test2005_22.C test2005_23.C test2005_24.C
  test2005_25.C test2005_26.C test2005_27.C test2005_28.C test2005_29.C
  test2005_30.C test2005_31.C test2005_32.C test2005_33.C test2005_34.C
  test2005_35.C test2005_36.C test2005_37.C test2005_38.C test2005_39.C
  test2005_40.C test2005_41.C test2005_42.C test2005_43.C test2005_44.C
  test2005_45.C test2005_46.C test2005_47.C test2005_48.C test2005_49.C
  test2005_50.C test2005_51.C test2005_52.C test2005_53.C test2005_55.C
  test2005_57.C test2005_58.C test2005_59.C test2005_60.C test2005_61.C
  test2005_62.C test2005_63.C test2005_64.C test2005_65.C test2005_66.C
  test2005_67.C test2005_68.C test2005_70.C test2005_71.C test2005_72.C
  test2005_73.C test2005_74.C test2005_75a.C test2005_75b.C test2005_76.C
  test2005_77.C test2005_78.C test2005_79.C test2005_80.C test2005_81.C
  test2005_82.C test2005_83.C test2005_84.C test2005_86.C test2005_87.C
  test2005_89.C test2005_90.C test2005_91.C test2005_92.C test2005_93.C
  test2005_94.C test2005_95.C test2005_96.C test2005_97.C test2005_98.C
  test2005_99.C test2005_100.C test2005_101.C test2005_102.C test2005_103.C
  test2005_104.C test2005_105.C test2005_106.C test2005_107.C test2005_108.C
  test2005_109.C test2005_110.C test2005_111.C test2005_112.C test2005_113.C
  test2005_114.C test2005_116.C test2005_117.C test2005_118.C test2005_119.C
  test2005_120.C test2005_121.C test2005_122.C test2005_123.C test2005_124.C
  test2005_125.C test2005_126.C test2005_127.C test2005_128.C test2005_129.C
  test2005_130.C test2005_131.C test2005_132.C test2005_133.C test2005_134.C
  test2005_135.C test2005_136.C test2005_137.C test2005_138.C test2005_139.C
  test2005_140.C test2005_141.C test2005_142.C test2005_143.C test2005_144.C
  test2005_145.C test2005_146.C test2005_147.C test2005_148.C test2005_149.C
  test2005_150.C test2005_152.C test2005_153.C test2005_154.C test2005_155.C
  test2005_156.C test2005_157.C test2005_158.C test2005_159.C test2005_160.C
  test2005_161.C test2005_162.C test2005_163.C test2005_164.C test2005_165.C
  test2005_166.C test2005_167.C test2005_169.C test2005_170.C test2005_171.C
  test2005_173.C test2005_174.C test2005_175.C test2005_176.C test2005_177.C
  test2005_178.C test2005_179.C test2005_180.C test2005_181.C test2005_182.C
  test2005_183.C test2005_184.C test2005_185.C test2005_187.C test2005_189.C
  test2005_190.C test2005_191.C test2005_192.C test2005_193.C test2005_194.C
  test2005_195.C test2005_196.C test2005_197.C test2005_198.C test2005_199.C
  test2005_202.C test2005_203.C test2005_206.C test2005_207.C test2005_208.C
  test2005_209.C test2006_01.C test2006_02.C test2006_03.C test2006_04.C
  test2006_05.C test2006_06.C test2006_07.C test2006_08.C test2006_10.C
  test2006_12.C test2006_13.C test2006_14.C test2006_16.C test2006_17.C
  test2006_18.C test2006_19.C test2006_20.C test2006_22.C test2006_23.C
  test2006_25.C test2006_26.C test2006_28.C test2006_34.C test2006_37.C
  test2006_38.C test2006_39.C test2006_40.C test2006_41.C test2006_42.C
  test2006_46.C test2006_47.C test2006_50.C test2006_52.C test2006_53.C
  test2006_54.C test2006_55.C test2006_56.C test2006_57.C test2006_58.C
  test2006_59.C test2006_61.C test2006_62.C test2006_63.C test2006_66.C
  test2006_67.C test2006_68.C test2006_69.C test2006_70.C test2006_71.C
  test2006_72.C test2006_73.C test2006_74.C test2006_75.C test2006_76.C
  test2006_77.C test2006_78.C test2006_79.C test2006_80.C test2006_81.C
  test2006_82.C test2006_83.C test2006_84.C test2006_85.C test2006_86.C
  test2006_87.C test2006_88.C test2006_89.C test2006_90.C test2006_91.C
  test2006_92.C test2006_93.C test2006_94.C test2006_95.C test2006_96.C
  test2006_97.C test2006_98.C test2006_99.C test2006_101.C test2006_102.C
  test2006_103.C test2006_104.C test2006_105.C test2006_106.C test2006_107.C
  test2006_108.C test2006_109.C test2006_111.C test2006_112.C test2006_113.C
  test2006_115.C test2006_116.C test2006_117.C test2006_118.C test2006_119.C
  test2006_120.C test2006_121.C test2006_122.C test2006_123.C test2006_124.C
  test2006_125.C test2006_126.C test2006_129.C test2006_139.C test2006_140.C
  test2006_141.C test2006_142.C test2006_143.C test2006_144.C test2006_145.C
  test2006_146.C test2006_147.C test2006_148.C test2006_149.C test2006_150.C
  test2006_151.C test2006_152.C test2006_153.C test2006_154.C test2006_155.C
  test2006_156.C test2006_157.C test2006_158.C test2006_159.C test2006_160.C
  test2006_161.C test2006_162.C test2006_163.C test2006_164.C test2006_165.C
  test2006_166.C test2006_180.C test_int_lexemes.C
  test_int_lexemes_donot_pass.C test_simple_int.C test_wchars.C
  lexPhase2003_01.C math.C inputForLoopLocator.C)

set(TESTCODE_INCLUDES -I${CMAKE_SOURCE_DIR}/tests/nonsmoke/functional/CompileTests/A++Code)

foreach(file_to_test ${TESTCODES})
	configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../Cxx_tests/${file_to_test}
    ${CMAKE_CURRENT_BINARY_DIR}/${file_to_test}
    COPYONLY)

  add_test(
    NAME SOURCEPOSITIONTEST_${file_to_test}
    COMMAND statements ${ROSE_FLAGS} ${TESTCODE_INCLUDES}
      -I${CMAKE_CURRENT_SOURCE_DIR}/../Cxx_tests
      -c ${CMAKE_CURRENT_BINARY_DIR}/${file_to_test})
  set_tests_properties(SOURCEPOSITIONTEST_${file_to_test} PROPERTIES LABELS SOURCEPOSITIONTEST)
endforeach()
