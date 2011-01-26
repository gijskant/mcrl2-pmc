# Replacements for mcrl2 parser
if( "${PARSER}" STREQUAL mcrl2 )
	message(STATUS "Post-processing mCRL2-parser")
  file(READ "mcrl2parser.cpp" MCRL2PARSER)
	string(REPLACE "include \"mcrl2parser.h\"" "include \"mcrl2/core/detail/mcrl2parser.h\"" MCRL2PARSER "${MCRL2PARSER}")
	file(WRITE "mcrl2parser.cpp" "${MCRL2PARSER}")

	message(STATUS "Post-processing mCRL2-lexer")
  file(READ "mcrl2lexer.cpp" MCRL2LEXER)
	string(REGEX REPLACE "\n[^\n]*isatty[^\n]*\n" "" MCRL2LEXER "${MCRL2LEXER}")
	file(WRITE "mcrl2lexer.cpp" "${MCRL2LEXER}")
endif( "${PARSER}" STREQUAL mcrl2 )

# Replacements for chi parser
if( "${PARSER}" STREQUAL chi )
	message(STATUS "Post-processing Chi-lexer")
  file(READ "chilexer.cpp" CHILEXER)
	string(REGEX REPLACE "\n[^\n]*isatty[^\n]*\n" "" CHILEXER "${CHILEXER}")
	file(WRITE "chilexer.cpp" "${CHILEXER}")
endif( "${PARSER}" STREQUAL chi )

# Replacements for lysa parser
if( "${PARSER}" STREQUAL lysa )
	message(STATUS "Post-processing Lysa-lexer")
  file(READ "lysalexer.cpp" CHILEXER)
	string(REGEX REPLACE "\n[^\n]*isatty[^\n]*\n" "" LYSALEXER "${LYSALEXER}")
	file(WRITE "lysalexer.cpp" "${CHILEXER}")
endif( "${PARSER}" STREQUAL lysa )

# Replacements for fsm parser
if( "${PARSER}" STREQUAL fsm )
	message(STATUS "Post-processing FSM-parser")
  file(READ "liblts_fsmparser.cpp" FSMPARSER)
	string(REPLACE "include \"liblts_fsmparser.h\"" "include \"mcrl2/lts/detail/liblts_fsmparser.h\"" FSMPARSER "${FSMPARSER}")
	file(WRITE "liblts_fsmparser.cpp" "${FSMPARSER}")

	message(STATUS "Post-processing FSM-lexer")
  file(READ "liblts_fsmlexer.cpp" FSMLEXER)
	string(REGEX REPLACE "\n[^\n]*isatty[^\n]*\n" "" FSMLEXER "${FSMLEXER}")
	file(WRITE "liblts_fsmlexer.cpp" "${FSMLEXER}")
endif( "${PARSER}" STREQUAL fsm )

# Replacements for dot parser
if( "${PARSER}" STREQUAL dot )
	message(STATUS "Post-processing DOT-parser")
  file(READ "liblts_dotparser.cpp" DOTPARSER)
	string(REPLACE "include \"liblts_dotparser.h\"" "include \"mcrl2/lts/detail/liblts_dotparser.h\"" DOTPARSER "${DOTPARSER}")
	file(WRITE "liblts_dotparser.cpp" "${DOTPARSER}")

	message(STATUS "Post-processing DOT-lexer")
  file(READ "liblts_dotlexer.cpp" DOTLEXER)
	string(REGEX REPLACE "\n[^\n]*isatty[^\n]*\n" "" DOTLEXER "${DOTLEXER}")
	file(WRITE "liblts_dotlexer.cpp" "${DOTLEXER}")
endif( "${PARSER}" STREQUAL dot )
