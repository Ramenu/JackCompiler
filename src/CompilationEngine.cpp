#include "CompilationEngine.h"
#include "FileReader.h"

//Class constructor
CompilationEngine::CompilationEngine(const char* inputFile, const char* outputFile)
{
    out = openOutputFile(outputFile); //Open the input .jack file
    tokenizer.in = openFile(inputFile); //Open the output .xml file
    //Initialize lineNum and count to 0
    tokenizer.lineNum = 0;
    tokenizer.count = 0;
    tokenIsStatement = false;
    tokenizer.endOfFileReached = false;

    tokenizer.advance(); //Advance and get the first line of the file
    compileClass(); //Compile class is mandatory
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

//Updates the current token, checks if any more tokens are available in the line, and if so outputs it
void CompilationEngine::getTokenAndOutput()
{
    updateTokenType();
    output(tokenType);
}

void CompilationEngine::updateTokenType()
{
    tokenType = tokenizer.getTokenType();
}


/* Methods for outputting identifiers, symbols, and keywords */

void CompilationEngine::output(std::string tokenType)
{
    if (tokenType == "KEYWORD")
        out << "<keyword> " << tokenizer.token << " </keyword>\n";
    else if (tokenType == "SYMBOL")
        out << "<symbol> " << tokenizer.token << " </symbol>\n";
    else
        out << "<identifier> " << tokenizer.token << " </identifier>\n";
}

/* End of methods */

//Compiles a statement (if, let, do, while, return)
void CompilationEngine::compileStatements()
{
    tokenIsStatement = true;
    out << "<statements>\n";
    while (true) //Loop until the token is not a statement or statement/function or method ends
    {
        if (!tokenIsStatement || tokenizer.token == "}" || tokenizer.line == "}")
            break;
        if (!tokenizer.hasMoreTokens()) //If no tokens are left in the line, advance and update the token
        {
            tokenizer.advance();
            updateTokenType();
        }
        if (tokenizer.line == "}")
            break;
        selectCompilationTask();
    }
    out << "</statements>\n";
}

//Based on the token type and token calls the appropriate method
void CompilationEngine::selectCompilationTask()
{
    if (tokenType == "KEYWORD") //Depending on the keyword will call the appropriate method
    {
        if (tokenizer.token == "if")
        {
            if (!tokenIsStatement)
                return compileStatements();
            return compileIf();
        }
        else if (tokenizer.token == "while")
        {
            if (!tokenIsStatement)
                return compileStatements();
            return compileWhile();
        }
        else if (tokenizer.token == "let")
        {
            if (!tokenIsStatement)
                return compileStatements();
            return compileLet();
        }
        else if (tokenizer.token == "do")
        {
            if (!tokenIsStatement)
                return compileStatements();
            return compileDo();
        }
        else if (tokenizer.token == "return")
        {
            if (!tokenIsStatement)
                compileStatements();
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
    }
    else if (tokenType == "IDENTIFIER")
    {
        out << "<identifier> " << tokenizer.token << " </identifier>\n";
        updateTokenType();
    }
    else
    {
        out << "<symbol> " << tokenizer.token << " </symbol>\n";
        updateTokenType();
        if (tokenizer.token == "=")
        {
            compileExpression(); //Compile it as an expression
        }
    }
}

//Calls the appropriate compile method depending on the token
void CompilationEngine::compileNext()
{
    while (tokenizer.in && tokenizer.in.peek() != EOF) //Continue looping until the tokenizer reaches the end of the file
    {
        if (!tokenizer.hasMoreTokens()) //Check if there are any more tokens in the line, if so, advance to the next line
        {
            tokenizer.advance();
            if (tokenizer.endOfFileReached) //If advance cannot find any more lines end the loop
                break;
        }
        if (tokenizer.lineNum == 59)
        {
            printf("rip\n");
        }
        updateTokenType();
        selectCompilationTask();
        continue;
    }
}

//Outputs each token while the line has tokens
void CompilationEngine::parseLineAndOutput()
{
    while (tokenizer.hasMoreTokens())
        getTokenAndOutput();
}


/* Contains compile statement methods */

void CompilationEngine::compileIf()
{
    tokenIsStatement = true;
    out << "<ifStatement>\n<keyword> if </keyword>\n";
    getTokenAndOutput(); //Should be "("

    updateTokenType(); //The next token
    compileExpression();

    getTokenAndOutput(); //Should be ")"
    getTokenAndOutput(); //Make sure the If statement starts with a curly brace "{"
    tokenIsStatement = false; //Since after "{" new statements can appear, put it to false
    if (tokenizer.token == "{")
    {
        while (true) 
        {   
            if (!tokenizer.hasMoreTokens())
                tokenizer.advance(); //If no tokens are found, advance
            updateTokenType();
            if (tokenizer.token == "}")
                break;
            selectCompilationTask();
        }
    }
    if (tokenizer.token == "}")
        out << "<symbol> } </symbol>\n</ifStatement>\n";
}

void CompilationEngine::compileWhile()
{
    tokenIsStatement = true;
    out << "<whileStatement>\n<keyword> while </keyword>\n";
    getTokenAndOutput(); //Should be "("

    updateTokenType(); //The next token (expression)
    compileExpression(); 

    getTokenAndOutput(); //Should be ")"
    while (tokenizer.token != "}")
    {
        if (!tokenizer.hasMoreTokens())
            tokenizer.advance(); //If no tokens are found, advance
        updateTokenType();
        selectCompilationTask();
    }
    out << "</whileStatement>\n";
}

void CompilationEngine::compileDo()
{
    tokenIsStatement = true;
    out << "<doStatement>\n<keyword> do </keyword>\n";
    while (tokenizer.token != "(")
        getTokenAndOutput();
    updateTokenType();
    compileExpressionList();
    getTokenAndOutput(); //Output the ")" if there is any
    out << "</doStatement>\n"; 
}

void CompilationEngine::compileReturn()
{
    tokenIsStatement = true;
    out << "<returnStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateTokenType();
    if (tokenType != "SYMBOL") //If it returns nothing (no token after return) then do not compile an expression
    {
        compileExpression();
        updateTokenType();
        output(tokenType); //Output ";"
    }
    else
        if (tokenizer.token == ";")
            output(tokenType);       
    out << "</returnStatement>\n";
}

void CompilationEngine::compileLet()
{
    tokenIsStatement = true;
    out << "<letStatement>\n<keyword> let </keyword>\n";
    while (tokenizer.token != "=")
        getTokenAndOutput();
    updateTokenType();
    compileExpression();
    getTokenAndOutput(); //Output the ";"
    out << "</letStatement>\n";
}
/* End of methods */

/* Methods that compile function/method declarations, class declarations, field, static, and variable declarations */

void CompilationEngine::compileClass()
{
    tokenIsStatement = false;
    if (tokenizer.getTokenType() == "KEYWORD")
        out << "<class>\n<keyword> " << tokenizer.token << " </keyword>\n";
    parseLineAndOutput();
    compileNext();
    out << "</class>";
}

void CompilationEngine::compileClassVarDec()
{
    tokenIsStatement = false;
    out << "<classVarDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    parseLineAndOutput();
    out << "</classVarDec>\n";
}

void CompilationEngine::compileSubroutineDec()
{
    tokenIsStatement = false;
    out << "<subroutineDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    while (tokenizer.token != "(")
        getTokenAndOutput();
    compileParameterList();
    compileSubroutineBody();
    out << "</subroutineDec>\n";
}

void CompilationEngine::compileParameterList()
{
    tokenIsStatement = false;
    out << "<parameterList>\n";
    updateTokenType(); //Check if the next token is a )
    while (tokenizer.token != ")")
        getTokenAndOutput();
    out << "</parameterList>\n";
    output(tokenType); //End the parameter list with a ")"
    updateTokenType(); //Get the next token which should be a "{"
}

void CompilationEngine::compileSubroutineBody()
{
    tokenIsStatement = false;
    if (tokenizer.token == "{") //Make sure that the subroutine starts with a curly brace
        out << "<subroutineBody>\n<symbol> { </symbol>\n";
    while (true) //While the subroutine does not end
    {
        //Keep advancing, and continue to compile
        
        if (!tokenizer.hasMoreTokens())
            tokenizer.advance();
        updateTokenType();
        if (tokenizer.line == "}" || tokenizer.token == "}") //Stop the loop when the function ends with a "}"
            break;
        selectCompilationTask();
        //Check to make sure after compilation, a curly brace was found
        //this is to make it so the tokenizer does not continue to advance even when the func/method is supposed to end..
        //which would cause it to be recursive
        if (tokenizer.line == "}" || tokenizer.token == "}") 
            break; 
    }
    //Make sure that the subroutine ends with a curly brace, as it should, although if it broke out of the loop
    //then it is most definitely a "}", but this is for extra precaution/error handling and may be removed in the final implementation
    if (tokenizer.token == "}" || tokenizer.line == "}") 
        out << "<symbol> } </symbol>\n</subroutineBody>\n";
}

void CompilationEngine::compileVarDec()
{
    tokenIsStatement = false;
    out << "<varDec>\n<keyword> var </keyword>\n";
    parseLineAndOutput();
    out  << "</varDec>\n";
}

/* End of methods */

/* Methods that compile expressions, and terms */

void CompilationEngine::compileExpression()
{
    out << "<expression>\n";
    compileTerm();
    out << "</expression>\n";
}

void CompilationEngine::compileExpressionList()
{
    out << "<expressionList>\n";
    while (tokenizer.token != ")") //while the expression list is not empty, if it is skips the loop
    {
        if (tokenizer.hasMoreTokens())
        {
            compileExpression();
            updateTokenType();
        }
    }
    out << "</expressionList>\n";
    output(tokenType);
}

void CompilationEngine::compileTerm()
{
    out << "<term>\n";
    output(tokenType);
    out << "</term>\n";
}

/* End of methods */
