add_library(luna
    CodeGenerate.cpp
    Function.cpp
    GC.cpp
    Lex.cpp
    LibAPI.cpp
    LibBase.cpp
    LibIO.cpp
    LibMath.cpp
    LibString.cpp
    LibTable.cpp
    ModuleManager.cpp
    Parser.cpp
    Runtime.cpp
    SemanticAnalysis.cpp
    State.cpp
    String.cpp
    StringPool.cpp
    SyntaxTree.cpp
    Table.cpp
    TextInStream.cpp
    Token.cpp
    Upvalue.cpp
    UserData.cpp
    Value.cpp
    VM.cpp
    )

add_executable(lunac
    Luna.cpp
    )

target_link_libraries(lunac
    luna
    )

set_target_properties(lunac
    PROPERTIES OUTPUT_NAME luna
    )
