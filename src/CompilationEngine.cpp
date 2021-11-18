#include "CompilationEngine.h"
#include "FileReader.h"

bool outputtedPrevious, doNotCallTerm, inExpressionList {};
//Class constructor
CompilationEngine::CompilationEngine(const char* inputFile, const char* outputFile, const char* inputFileName)
{
    out = openOutputFile(outputFile); //Open the input .jack file
    tokenizer.in = openFile(inputFile); //Open the output .xml file
    this->inputFileName = inputFileName; //Needed for outputting for where the compilation error happened
    //Initialize lineNum and count to 0
    tokenizer.lineNum = 0;
    tokenizer.count = 0;
    tokenIsStatement = false;
    tokenizer.endOfFileReached = false;
    errorHappened = false;
    errorsFound = 0;
    outputtedPrevious = true;
    isLL1 = true;

    tokenizer.advance(); //Advance and get the first line of the file
    updateToken(); //Get the first token, should be "class"!
    compileClass();
}

void CompilationEngine::compileFile()
{
    while (!tokenizer.endOfFileReached)
    {
        if (tokenizer.in && tokenizer.in.peek() == EOF) //If the file does not contain any more lines end the loop
            break;
        selectCompilationTask();
    }
    if (!errorHappened)
        printf("File %s successfully compiled! Woohoo! :)\n", inputFileName);
    else
        printf("Compilation failed in file %s with %d errors found\n", inputFileName, errorsFound);
}

//Opens the output file
std::ofstream CompilationEngine::openOutputFile(const char* filePath)
{
    std::ofstream of;
    of.open(filePath, std::ofstream::out | std::ofstream::trunc);
    if (of.fail())
        printf("ERROR: Failed to create/write to output file at %s.\n", filePath);
    return of;
}

//Advances to the next line if no tokens are found
void CompilationEngine::advanceIfNoTokens()
{
    if (!tokenizer.hasMoreTokens())
    {
        tokenizer.advance();
        updateToken(); //Update the token after advancing (must be done always whenever after advancing!)
    }
}

//Updates the current token, checks if any more tokens are available in the line, and if so outputs it
void CompilationEngine::getTokenAndOutput()
{
    updateToken();
    output(tokenType);
}

void CompilationEngine::updateToken()
{
    tokenType = tokenizer.getTokenType();
}

//Returns the number of statements that do not match with the current token
unsigned int CompilationEngine::noStatementMatches()
{
    const char* statementArr[5] {"if", "while", "do", "let", "return"}; //Array with statement declaration keywords
    unsigned int statementNotMatchCount {}; //boolean to check if the updated token is not a statement

    for (unsigned int i {}; i < 5; i++) //Iterate through array
        if (tokenizer.token != statementArr[i]) 
            statementNotMatchCount++;
    
    return statementNotMatchCount;
}

/* Methods for outputting identifiers, symbols, and keywords */

void CompilationEngine::output(std::string tokenType)
{
    std::string outputToken {tokenizer.token}; //By default, it is the current token
    if (!outputtedPrevious) //If it is LL2 and the previous token was not outputted yet
        outputToken = prevToken; //Let the token outputted be the previous token
    if (tokenType == "KEYWORD")
        out << "<keyword> " << outputToken << " </keyword>\n";
    else if (tokenType == "SYMBOL")
        out << "<symbol> " << outputToken << " </symbol>\n";
    else if (tokenType == "STRING_CONST")
        out << "<stringConstant> " << outputToken << " </stringConstant>\n";
    else if (tokenType == "INT_CONST")
        out << "<integerConstant> " << outputToken << " </integerConstant>\n";
    else
        out << "<identifier> " << outputToken << " </identifier>\n";
    out.flush();
}

/* End of methods */

//Based on the token type and token calls the appropriate method
void CompilationEngine::selectCompilationTask()
{
    if (tokenizer.token == "if")
    {
        if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
            return compileStatements();
        return compileIf();
    }
    else if (tokenizer.token == "while")
    {
        if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
            return compileStatements();
        return compileWhile();
    }
    else if (tokenizer.token == "let")
    {
        if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
            return compileStatements();
        return compileLet();
    }
    else if (tokenizer.token == "do")
    {
        if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
            return compileStatements();
        return compileDo();
    }
    else if (tokenizer.token == "return")
    {
        if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
            return compileStatements();
        return compileReturn();
    }
    else if (tokenizer.token == "function" || tokenizer.token == "method" || tokenizer.token == "constructor")
        return compileSubroutineDec();
    else if (tokenizer.token == "field" || tokenizer.token == "static")
        return compileClassVarDec();
    else if (tokenizer.token == "var")
        return compileVarDec();
    else if (tokenizer.token == "class")
        return compileClass();
    else if (tokenizer.token == "}") //Once the file reaches its end this will be selected
        tokenizer.endOfFileReached = true;
    else
        reportError("N/A"); //Add more to reportError later
}

//Continues to parse and compile the line until a symbol is found
void CompilationEngine::parseUntilSymbol(char symbol)
{
    while (tokenizer.token[0] != symbol) //While the current token does not match the symbol
    {
        updateToken(); //Update the token
        if (tokenizer.token[0] == symbol) //Extra precaution, check if the updated token is a symbol
            break;
        output(tokenType);
    }
}

//Parses the line and outputs it, used only for cases like field, var declarations where no other compilation methods are required
//Should be used only after a keyword!! Example in compileClass method
void CompilationEngine::parseLineAndOutput()
{
    while (tokenizer.hasMoreTokens())
    {
        updateToken();
        output(tokenType);
    }
}

bool CompilationEngine::subroutineStarts()
{
    if (tokenizer.token[0] == '{')
        return true;
    return false;
}

bool CompilationEngine::subroutineEnds()
{
    if (tokenizer.token[0] == '}')
        return true;
    return false;
}

/* Contains compile statement methods */

//Compiles a statement (if, let, do, while, return)
void CompilationEngine::compileStatements()
{
    tokenIsStatement = true;
    out << "<statements>\n";
    while (tokenIsStatement && !subroutineStarts() && !subroutineEnds())
    {
        selectCompilationTask(); 
    }
    out << "</statements>\n";
    tokenIsStatement = false;
    out.flush();
}

void CompilationEngine::compileIf()
{
    tokenIsStatement = true;
    out << "<ifStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    syntaxAnalyzer("(", "SYMBOL");
    updateToken();
    compileExpression();
    updateToken();
    syntaxAnalyzer(")", "SYMBOL");
    updateToken(); //Should be "{", very important to update it here, because compileStatements() will not be called if not done!
    syntaxAnalyzer("{", "SYMBOL");
    tokenIsStatement = false;
    if (tokenizer.hasMoreTokens())
        updateToken(); //Even though the if ends here, if body is on the same line update it
    else //Otherwise just advance to the body of the if statement
    {
        tokenizer.advance();
        updateToken();
    }

    if (noStatementMatches() == 5) //If the token is not any of the statements, output an empty statements body
        out << "<statements>\n</statements>\n";
    while (tokenizer.token != "}")
        selectCompilationTask();


    syntaxAnalyzer("}", "SYMBOL"); //Make sure the if statement ends with a brace "}"
    tokenIsStatement = true;
    advanceIfNoTokens();
    if (tokenizer.token == "else") //If the updated token contains an else statement, then do not end the if statement here
    {
        out << "<keyword> " << tokenizer.token << " </keyword>\n"; // <-- else
        updateToken();
        syntaxAnalyzer("{", "SYMBOL"); //Should start with a "{"
        tokenIsStatement = false;
        if (tokenizer.hasMoreTokens())
            updateToken();
        else 
        {
            tokenizer.advance();
            updateToken();
        }
        if (noStatementMatches() == 5)
            out << "<statements>\n</statements>\n";
        while (tokenizer.token != "}")
            selectCompilationTask(); // <-- body of else statement
        syntaxAnalyzer("}", "SYMBOL"); //Should end with a "}"
        tokenIsStatement = true;
        advanceIfNoTokens();
    }
    out << "</ifStatement>\n";    
}

void CompilationEngine::compileWhile()
{
    tokenIsStatement = true;
    out << "<whileStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    syntaxAnalyzer("(", "SYMBOL");
    updateToken();
    out << "<expression>\n";
    while (tokenizer.token != ")")
    {
        compileTerm();
        updateToken();
        if (tokenizer.token != ")" && tokenType == "SYMBOL") //Each term must be seperated by a symbol
        {
            output(tokenType);
            updateToken();
        }
    }
    out << "</expression>\n";
    syntaxAnalyzer(")", "SYMBOL");
    updateToken(); //Should be "{", very important to update it here, because compileStatements() will not be called if not done!
    syntaxAnalyzer("{", "SYMBOL");
    tokenIsStatement = false;
    if (tokenizer.hasMoreTokens())
        updateToken(); //Even though the while declaration ends here, if while body is on the same line update it
    else
    {
        tokenizer.advance();
        updateToken();
    }
    while (tokenizer.token != "}")
        selectCompilationTask();
    syntaxAnalyzer("}", "SYMBOL"); //Make sure the while statement ends with a brace "}"
    tokenIsStatement = true;
    advanceIfNoTokens();
    out << "</whileStatement>\n";
}

void CompilationEngine::compileDo()
{
    tokenIsStatement = true;
    out << "<doStatement>\n<keyword> " << tokenizer.token << " </keyword>\n"; 
    parseUntilSymbol('(');
    syntaxAnalyzer("(", "SYMBOL");
    compileExpressionList();
    syntaxAnalyzer(")", "SYMBOL");
    updateToken();
    syntaxAnalyzer(";", "SYMBOL");
    if (tokenizer.hasMoreTokens())
        updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
    else
        advanceIfNoTokens();
    out << "</doStatement>\n";
    out.flush();    
}

void CompilationEngine::compileReturn()
{
    tokenIsStatement = true;
    out << "<returnStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    if (tokenType == "IDENTIFIER" || tokenType == "STR_CONST" || tokenType == "INT_CONST")
    {
        compileExpression();
        updateToken(); //Get the ";"
        syntaxAnalyzer(";", "SYMBOL");
    }
    else
        syntaxAnalyzer(";", "SYMBOL");
    out << "</returnStatement>\n"; 
    if (tokenizer.hasMoreTokens())
        updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
    else
        advanceIfNoTokens();
    out.flush();
}

void CompilationEngine::compileLet()
{
    tokenIsStatement = true;
    out << "<letStatement>\n<keyword> " << tokenizer.token << " </keyword>\n" ;
    updateToken();
    output(tokenType);
    updateToken();
    if (tokenizer.token == "[") //Check to see if its accessing array elements
    {
        output(tokenType);
        updateToken();
        compileExpression();
        updateToken();
        syntaxAnalyzer("]", "SYMBOL");
        updateToken();
    }
    syntaxAnalyzer("=", "SYMBOL");
    updateToken();

    /* Since at this point the parsing process becomes LL2 instead of LL1 we have to store the tokentype and
       the token in temporary variables and get the next token and parse it appropriately based on that */

    prevToken = tokenizer.token; //Store the current token in a temporary token before updating it
    prevTokenType = tokenType; //Store current token type

    updateToken(); //Check to see if the updated token is a . ( or [
    outputtedPrevious = false;
    if (tokenizer.token == "." || tokenizer.token == "(" || tokenizer.token == "[") 
        isLL1 = false; //Set LL1 to false, so when compileExpression is called it will handle it appropriately
    compileExpression();
    outputtedPrevious = true; //Set it to true just for precaution

    if (tokenizer.hasMoreTokens())
        updateToken();
    syntaxAnalyzer(";", "SYMBOL");
    out << "</letStatement>\n"; 
    if (tokenizer.hasMoreTokens())
        updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
    else
        advanceIfNoTokens();
    out.flush(); 
}
/* End of methods */

/* Methods that compile function/method declarations, class declarations, field, static, and variable declarations */

void CompilationEngine::compileClass()
{
    tokenIsStatement = false;
    out << "<class>\n";
    syntaxAnalyzer("class", "KEYWORD");
    parseLineAndOutput(); //Parse the line after class
    advanceIfNoTokens();
    compileFile();
    syntaxAnalyzer("}", "SYMBOL"); //"}" is expected to end the class
    out << "</class>\n";
}

void CompilationEngine::compileClassVarDec()
{
    tokenIsStatement = false;
    out << "<classVarDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    parseLineAndOutput();
    out << "</classVarDec>\n";
    if (tokenizer.hasMoreTokens())
        updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
    else
        advanceIfNoTokens();
    out.flush();
}

void CompilationEngine::compileSubroutineDec()
{
    tokenIsStatement = false;
    out << "<subroutineDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    parseUntilSymbol('(');
    output(tokenType); //Output the "("
    compileParameterList();
    updateToken(); //Should be a "{" now
    if (tokenizer.token == "{")
        compileSubroutineBody();
    out.flush();
    out << "</subroutineDec>\n"; //No advance here because subroutine body is called which has to have a { as a token
}

void CompilationEngine::compileParameterList()
{
    tokenIsStatement = false;
    out << "<parameterList>\n";
    while (tokenizer.token != ")")
    {
        advanceIfNoTokens();
        parseUntilSymbol(')');
    }
    out << "</parameterList>\n";
    out.flush();
    output(tokenType); //Output the ")"
}

void CompilationEngine::compileSubroutineBody()
{
    tokenIsStatement = false;
    out << "<subroutineBody>\n";
    output(tokenType); //Output the "{" 
    while (tokenizer.token != "}") //While the subroutine does not end
    {
        advanceIfNoTokens(); //Gets the next line if no tokens are present in the line
        if (tokenizer.line.find("}") != std::string::npos) //Checks if the line ends
        {
            selectCompilationTask();
            break; //Break the loop once the "}" is found
        } 
        selectCompilationTask(); 
    }
    output(tokenType); //Output the "}"
    out << "</subroutineBody>\n";
    advanceIfNoTokens();
    out.flush();
}

void CompilationEngine::compileVarDec()
{
    tokenIsStatement = false;
    out << "<varDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    parseLineAndOutput();
    out << "</varDec>\n";
    if (tokenizer.hasMoreTokens())
        updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
    else
        advanceIfNoTokens();
    out.flush();
}

/* End of methods */

/* Methods that compile expressions, and terms */

void CompilationEngine::compileExpression()
{
    out << "<expression>\n";
    if (!doNotCallTerm) //if compileTerm() was already called then do not compile it again (avoids recursion)
    {
        compileTerm();
        if (inExpressionList)
        {
            updateToken();
        }
        //Since outputted previous was set to true no need to update token, unless we are in expression list
        if (tokenizer.token == "+" || tokenizer.token == "-" || tokenizer.token == "*" || tokenizer.token == "/") 
        {
            outputtedPrevious = true;
            while (tokenizer.token != ";" && tokenizer.token != ")")
            {
                output(tokenType);
                updateToken();
                if (tokenizer.token != ";" && tokenizer.token != ")")
                    compileTerm();
                updateToken();
            }
        }
    }
    else
    {
        out << "<term>\n";
        output(tokenType);
        out << "</term>\n";
        if (inExpressionList)
        {
            updateToken();
        }
    }
    out << "</expression>\n";
    out.flush();
}

void CompilationEngine::compileExpressionList()
{
    inExpressionList = true;
    out << "<expressionList>\n";
    updateToken(); //Get the first expression
    while (tokenizer.token != ")")
    {
        compileExpression();
        if (tokenizer.token != ")") //Make sure the expression list does not end
        {
            output(tokenType);
            updateToken();
        }
    }
    out << "</expressionList>\n";
    inExpressionList = false;
}

void CompilationEngine::compileTerm()
{
    out << "<term>\n";
    if (isLL1)
    {
        if (outputtedPrevious)
            output(tokenType);
        else
            output(prevTokenType);
        if (tokenizer.lineLeftToParse[1] == '[') //Check if next token is accessing array element
        {
            updateToken();
            output(tokenType);
            updateToken();
            compileExpression();
            updateToken();
            output(tokenType);
        }
    }
    else //Otherwise if we are in LL2
    {
        output(prevTokenType); //Output the previous token first
        outputtedPrevious = true;
        output(tokenType);
        updateToken();
        output(tokenType); //Output the "(" <--
        parseUntilSymbol('(');
        output(tokenType); //Output ")"
        doNotCallTerm = true;
        compileExpressionList();
        output(tokenType); //Output ")"
        //Set boolean values back to their original values
        doNotCallTerm = false;
        isLL1 = true;
    }
    out << "</term>\n";
    out.flush();
}

/* End of methods */

//Analyzes syntax and makes sure it matches up with the expected output parameter
void CompilationEngine::syntaxAnalyzer(const char* expectedOutput, const char* expectedTokenType)
{
    if (tokenType == expectedTokenType) //If the token type matches the expected type (e.g. SYMBOL == SYMBOL)
    {
        //If it is not an identifier (identifiers do not have keywords or characters in Jack!) so there is no expected output for them
        if (tokenType != "IDENTIFIER") 
        {
            if (tokenizer.token != expectedOutput)
                reportError(expectedOutput);
            else //If no error is found, outputs it to the xml file
                output(tokenType);
        }
        else 
            output(tokenType);
    }
}

//Reports an error with the expected token that was supposed to be placed
void CompilationEngine::reportError(const char* token)
{
    errorsFound++;
    errorHappened = true;
    printf("\nERROR: At line %d: Expected \"%s\"\n", tokenizer.lineNum, token);
}