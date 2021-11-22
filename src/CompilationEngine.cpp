#include "CompilationEngine.h"
#include "FileReader.h"
#define RESET   "\033[0m"
#define RED     "\033[31m" 

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
    errorHappened = false;
    errorsFound = 0;
    expressionCallCount = 0;
    inDo = false;
    table.noOfFieldVars = 0;
    table.noOfStaticVars = 0;

    tokenizer.advance(); //Advance and get the first line of the file
    updateToken(); //Get the first token, should be "class"!
    compileClass();
    out.close();
}

void CompilationEngine::compileFile()
{
    while (tokenizer.in.peek() != EOF) //While the file contains lines
        selectCompilationTask();
    
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
    //TODO: Add a built-in error handler to this
    std::string outputToken {tokenizer.token}; //By default, it is the current token

    switch (tokenizer.token[0]) //In .xml these characters are treated differently so these are just mnemonics for them
    {
        case '<': outputToken = "&lt;"; break;
        case '>': outputToken = "&gt;"; break;
        case '\"': outputToken = "&quot;"; break;
        case '&': outputToken = "&amp;"; break;
    }
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
        if (inSubroutine)
        {
            if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
                return compileStatements();
            return compileIf();
        }
        else
            reportError("if statement declaration outside of subroutine scope is not allowed", false);
    }
    else if (tokenizer.token == "while")
    {
        if (inSubroutine)
        {
            if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
                return compileStatements();
            return compileWhile();
        }
        else
            reportError("while statement declaration outside of subroutine scope is not allowed", false);
    }
    else if (tokenizer.token == "let")
    {
        if (inSubroutine)
        {
            if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
                return compileStatements();
            return compileLet();
        }
        else
            reportError("let statement declaration outside of subroutine scope is not allowed", false);
    }
    else if (tokenizer.token == "do")
    {
        if (inSubroutine)
        {
            if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
                return compileStatements();
            return compileDo();
        }
        else
            reportError("do statement declaration outside of subroutine scope is not allowed", false);
    }
    else if (tokenizer.token == "return")
    {
        if (inSubroutine)
        {
            if (!tokenIsStatement || subroutineStarts() || subroutineEnds())
                return compileStatements();
            return compileReturn();
        }
        else
            reportError("return statement declaration outside of subroutine scope is not allowed", false);
    }
    else if (tokenizer.token == "function" || tokenizer.token == "method" || tokenizer.token == "constructor")
        return compileSubroutineDec();
    else if (tokenizer.token == "field" || tokenizer.token == "static")
        return compileClassVarDec();
    else if (tokenizer.token == "var")
        if (inSubroutine)
            return compileVarDec();
        else
            reportError("variable declaration outside of subroutine scope is not allowed", false);
    else if (tokenizer.token == "class")
        return compileClass();
    else
        return tokenizer.toNextLine(); //Why not replace this with advance? Because the purpose of this method is to check if the file has ended
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
    while (tokenizer.hasMoreTokens() && tokenizer.lineLeftToParse.length() != 0)
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
    bool startsExpression {syntaxAnalyzer("(")};
    if (startsExpression)
    {
        updateToken();
        compileExpression();
        bool endsExpression {syntaxAnalyzer(")")};
        if (endsExpression)
        {
            //Should be "{", very important to update it here, because compileStatements() will not be called if not done!
            if (tokenizer.hasMoreTokens())
                updateToken(); 
            else
                advanceIfNoTokens();
            bool bodyStarts {syntaxAnalyzer("{")};
            if (bodyStarts)
            {
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

                syntaxAnalyzer("}"); //Make sure the if statement ends with a brace "}"
                tokenIsStatement = true;
                advanceIfNoTokens();
                if (tokenizer.token == "else") //If the updated token contains an else statement, then do not end the if statement here
                {
                    out << "<keyword> " << tokenizer.token << " </keyword>\n"; // <-- else
                    if (tokenizer.hasMoreTokens())
                        updateToken();
                    else
                        advanceIfNoTokens();
                    if (tokenizer.lineLeftToParse.length() == 0) //Double check since "else" is not of length 1
                    {
                        tokenizer.advance();
                        updateToken();
                    }
                    bool elseBodyStarts {syntaxAnalyzer("{")}; //Should start with a "{"
                    if (elseBodyStarts)
                    {
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
                        syntaxAnalyzer("}"); //Should end with a "}"
                    }
                    tokenIsStatement = true;
                    advanceIfNoTokens();
                }
            }
        }
    }
    out << "</ifStatement>\n";    
}

void CompilationEngine::compileWhile()
{
    tokenIsStatement = true;
    out << "<whileStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    bool startsExpression {syntaxAnalyzer("(")};
    if (startsExpression)
    {
        updateToken();
        compileExpression();
        bool expressionEnds {syntaxAnalyzer(")")};
        if (expressionEnds)
        {
            /* This while loop is important here, since compileExpression() parses up to ")" each time it is called, if there are multiple
               brackets then it will not update those tokens but it will output them, so updating it until it reaches the body should do */
            bool bodyStarts {};
            advanceIfNoTokens(); //In case ")" is the last token of the line and "{" is on the next line
            if (tokenizer.lineLeftToParse.find("{") != std::string::npos)
            {
                bodyStarts = true;
                while (tokenizer.token != "{")
                    updateToken(); //Should be "{", very important to update it here, because compileStatements() will not be called if not done!
            }
            else
            {
                bodyStarts = false;
                reportError("{", true);
            }
            if (bodyStarts)
            {
                syntaxAnalyzer("{");
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
                syntaxAnalyzer("}"); //Make sure the while statement ends with a brace "}"
            }
        }
    }
    tokenIsStatement = true;
    advanceIfNoTokens();
    out << "</whileStatement>\n";
}

void CompilationEngine::compileDo()
{
    bool hasExpressionList {};
    tokenIsStatement = true;
    out << "<doStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    if (tokenizer.line.find("(") != std::string::npos)
    {
        hasExpressionList = true;
        parseUntilSymbol('(');
    }
    else
        reportError("Do statement does not have an expression list\n", false);
    if (hasExpressionList)
    {
        bool startsExpressionList {syntaxAnalyzer("(")};
        if (startsExpressionList)
        {
            inExpressionList = true;
            inDo = true; //The purpose of "inDo" is to let the program know that expression list is being called before compiling expression
            compileExpressionList();
            inExpressionList = false;
            inDo = false;
            updateToken();
            bool doEnds {syntaxAnalyzer(";")};
            if (doEnds)
            {
                if (tokenizer.hasMoreTokens())
                    updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
                else
                    advanceIfNoTokens();
            }
        }
    }
    
    out << "</doStatement>\n";
    out.flush();    
}

void CompilationEngine::compileReturn()
{
    tokenIsStatement = true;
    out << "<returnStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    if (tokenType == "IDENTIFIER" || tokenType == "STR_CONST" || tokenType == "INT_CONST" || tokenType == "KEYWORD")
        compileExpression();
    bool returnEnded {syntaxAnalyzer(";")}; //Output ";"
    out << "</returnStatement>\n"; 
    if (returnEnded)
    {
        if (tokenizer.hasMoreTokens())
            updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
        else
            advanceIfNoTokens();
    }
    out.flush();
}

void CompilationEngine::compileLet()
{
    bool accessArray {}, arrayExpEnds {};
    tokenIsStatement = true;
    out << "<letStatement>\n<keyword> " << tokenizer.token << " </keyword>\n" ;
    updateToken();
    output(tokenType); //Output the identifier
    updateToken();
    if (tokenizer.token == "[") //Check to see if its accessing array elements
    {
        accessArray = true;
        output(tokenType);
        updateToken();
        compileExpression();
        arrayExpEnds = syntaxAnalyzer("]");
        updateToken();
    }
    bool hasEquals {syntaxAnalyzer("=")};
    if (!accessArray || accessArray && arrayExpEnds)
    {
        if (hasEquals)
        {
            updateToken();
            compileExpression();

            //If the token is a semicolon then the let statement ends here, otherwise update to see if it is a ";"
            if (tokenizer.line.find(";") != std::string::npos)
                if (tokenizer.token != ";" && tokenizer.hasMoreTokens()) 
                    updateToken();
            bool letEnds {syntaxAnalyzer(";")};
            if (letEnds)
            {
                out << "</letStatement>\n"; 
                if (tokenizer.hasMoreTokens())
                    updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
                else
                    advanceIfNoTokens();
                out.flush(); 
            }
        }
    }
    
}
/* End of methods */

/* Methods that compile function/method declarations, class declarations, field, static, and variable declarations */

void CompilationEngine::compileClass()
{
    tokenIsStatement = false;
    out << "<class>\n";
    syntaxAnalyzer("class");
    updateToken(); //Get the identifier
    output(tokenType); 
    if (tokenizer.line.find("{") != std::string::npos) //If it finds the class body declaration then it is safe to update the token
        updateToken();
    else //Otherwise it may be on the next line
    {
        tokenizer.advance();
        updateToken();
    }
    if (tokenizer.token == "{") //Check if class body is on the next line
    {
        output(tokenType);
        tokenizer.advance();
        updateToken();
    }
    compileFile();
    syntaxAnalyzer("}"); //"}" is expected to end the class
    out << "</class>\n";
}

void CompilationEngine::compileClassVarDec()
{
    bool endsAfterComma {};
    tokenIsStatement = false;
    if (tokenizer.line.find(";") != std::string::npos)
    {
        out << "<classVarDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
        std::string identifierKind {tokenizer.token}; //Get the kind of the identifier (either "static" OR "field")
        updateToken();
        output(tokenType);
        std::string identifierType {tokenizer.token}; //Get the identifier type (int, string, etc..)
        updateToken();
        output(tokenType);
        table.define(tokenizer.token, identifierType, identifierKind); //The class table will add the newly defined identifier
        updateToken();

        while (tokenizer.token != ";") //Check to see if there are more variables being declared
        {
            if (!syntaxAnalyzer(",")) //Each variable declared must be seperated with a comma
                break;
            updateToken();
            endsAfterComma = tokenizer.token == ";";
            if (endsAfterComma) //If the declaration ends after a comma, throw an error
            {
                reportError("Expected an identifier after \",\"", false);
                break;
            }
            output(tokenType); //First output the identifier
            table.define(tokenizer.token, identifierType, identifierKind); //Define the identifier in the class table
            updateToken(); //Get "," or ";"
        }
        if (!endsAfterComma)
        {
            output(tokenType); //Output ";"
            out << "</classVarDec>\n";
            if (tokenizer.hasMoreTokens())
                updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
            else
                advanceIfNoTokens();
            out.flush();
        }
    }
    else
        reportError("class variable declaration does not end with a \";\"", false);
}

void CompilationEngine::compileSubroutineDec()
{
    table.startSubroutine(); //Clears the sub routine tables contents
    tokenIsStatement = false;
    out << "<subroutineDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    bool subroutineHasPara {};
    if (tokenizer.line.find("(") != std::string::npos)
    {
        subroutineHasPara = true;
        parseUntilSymbol('(');
    }
    else
        reportError("Expected a \"(\" for subroutine parameter list", false);
    if (subroutineHasPara)
    {
        syntaxAnalyzer("("); //Output the "("
        compileParameterList();
        if (tokenizer.hasMoreTokens()) //Check if there are more tokens in case ")" was the last token
            updateToken(); //Should be a "{" now
        else
            advanceIfNoTokens(); 
        if (tokenizer.token == "{")
            compileSubroutineBody();
        out.flush();
        out << "</subroutineDec>\n"; //No advance here because subroutine body is called which has to have a { as a token
    }
}

void CompilationEngine::compileParameterList()
{
    tokenIsStatement = false;
    out << "<parameterList>\n";
    updateToken(); //Check the first parameter, or if the parameter ends
    if (tokenizer.line.find(")") != std::string::npos)
    {
        while (tokenizer.token != ")")
        {
            output(tokenType); //Output the token type
            updateToken();
            if (tokenizer.token == ")") //If the parameter list ends inappropriately (right before an identifier declaration) throw an error
            {
                syntaxAnalyzer("IDENTIFIER");
                break;
            }
            output(tokenType);
            table.define(tokenizer.token, tokenizer.prevToken, "argument"); //Adds an argument variable to the subroutine table
            updateToken(); 
            
            //Check if the parameter list ends, if not each parameter must be seperated by a comma
            if (tokenizer.token != ")")
            {
               if (!syntaxAnalyzer(",")) //Each variable must be seperated by a ","
                   break;
                updateToken(); //Repeat..
            }
        }
        out << "</parameterList>\n";
        out.flush();
        output(tokenType); //Output the ")"
    }
    else
        reportError("Parameter list does not end with a \")\"", false);
}

void CompilationEngine::compileSubroutineBody()
{
    inSubroutine = true;
    tokenIsStatement = false;
    out << "<subroutineBody>\n";
    bool subroutineBodyStarts {syntaxAnalyzer("{")};
    if (subroutineBodyStarts)
    {
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
    }
    bool subroutineEnds {syntaxAnalyzer("}")};
    if (subroutineEnds)
    {
        out << "</subroutineBody>\n";
        advanceIfNoTokens();
        out.flush();
    }
    inSubroutine = false;
}

void CompilationEngine::compileVarDec()
{
    bool improperVarDec {};
    tokenIsStatement = false;
    out << "<varDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    if (inSubroutine) //Variables cannot be declared outside of subroutines
    {
        if (tokenizer.line.find(";") != std::string::npos) //To avoid infinite loop
        {
            updateToken(); //Get the identifier type (int, string, classname, etc..)
            std::string identifierType {tokenizer.token};
            output(tokenType);
            updateToken(); //Get the first identifier
            std::string identifierName {tokenizer.token};
            output(tokenType);
            table.define(identifierName, identifierType, "local");
            updateToken(); //Check if its "," or ";"
            //Since for variable declarations, identifier names can be seperated with commas, keep adding every var to the symbol table
            while (tokenizer.token != ";")
            {
                improperVarDec = !syntaxAnalyzer(","); //Make sure each variable declared is seperated by a comma
                updateToken();
                if (tokenizer.token != ";") //If there is a semicolon right after a comma, that is incorrect, punish the programmer
                {
                    output(tokenType);
                    table.define(tokenizer.token, identifierType, "local"); 
                    updateToken();
                }
                else
                {
                    improperVarDec = true;
                    reportError("\";\" found after \",\" expected an identifier", false);
                }
            }
            syntaxAnalyzer(";");
            if (!improperVarDec)
            {
                out << "</varDec>\n";
                if (tokenizer.hasMoreTokens())
                    updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
                else
                    advanceIfNoTokens();
                out.flush();
            }
        }
        else
            reportError("Variable declaration does not end with a \";\"", false);
    }
    else
        reportError("Variable declaration outside of subroutine scope is not allowed", false);
    
}

/* End of methods */

/* Methods that compile expressions, and terms */

void CompilationEngine::compileExpression()
{
    expressionCallCount++; //Increment each time compile expression is called
    bool contLoop {};
    char operatorSymbols[9] {'+', '-', '*', '/', '&', '|', '<', '>', '='};
    char endExpressionSymbols[4] {')', ']', ';', ','}; //Symbols that an expression can end with
    out << "<expression>\n";
   
    while (true)
    {
        contLoop = false; //Reinitialize contLoop to false once/if the loop is repeated again
        compileTerm(); //The token is updated when the term is compiled, so no need to compile it here
        for (unsigned short i {}; i < 9; i++)
        {
            if (tokenizer.token[0] == operatorSymbols[i])
            {
                output(tokenType); //Output an operator
                updateToken();
                contLoop = true; //Repeat and compile a term again
                break;
            }
        }
        if (contLoop)
            continue;
        for (unsigned short i {}; i < 4; i++)
            if (tokenizer.token[0] == endExpressionSymbols[i])
                break; 
        //TODO: Output error message if there is no end expression 
        break;
    }
    out << "</expression>\n";
    out.flush();

    /* If there is more than one expression being compiled and the expression finishes, we want to update the token
       since the expression does not actually end yet */
    if (tokenizer.token == ")" || tokenizer.token == "]")
    {
        //If we are not compiling do, then expression calls expression list, ECP must be over or equal to 2 to be updated
        if (inExpressionList && !inDo && expressionCallCount > 2) 
        {
            output(tokenType);
            updateToken();
        }
        //Since compiling do calls compile expression list ECP would have to be over 1 to be updated
        else if (inExpressionList && inDo && expressionCallCount > 1)
        {
            output(tokenType);
            updateToken();
        }
        else if (!inExpressionList && expressionCallCount > 1)
        {
            output(tokenType);
            updateToken();
        }
    }
    expressionCallCount--; //Decrement when the method finishes, should be 0 when there is expression being compiled in the call stack
}

void CompilationEngine::compileExpressionList()
{
    out << "<expressionList>\n";
    updateToken(); //Get the first expression
    while (tokenizer.token != ")" && tokenizer.token != ";")
    {
        compileExpression();
        //The expression list must end with a parenthesis, and each expression, each time compiled, must be seperated with a comma
        if (tokenizer.token != ")" && tokenizer.token == ",") 
        {
            bool isComma {syntaxAnalyzer(",")};
            if (!isComma)
                break;
            updateToken();
        }
    }
    out << "</expressionList>\n";
    output(tokenType); //Output ")"
}

void CompilationEngine::compileTerm()
{
    
    out << "<term>\n";
    output(tokenType);
    if (!compileAnotherExpressionOrTerm(false)) //Check if the current token starts an expression or term, if it returns false..
    {
        updateToken(); //Update the token
        compileAnotherExpressionOrTerm(true); //Check if this token starts an expression or term
    }
    if (tokenizer.token == ".") //Check if accessing class/object
    {
        bool expressionListStarts {};
        output(tokenType);
        if (tokenizer.line.find("(") != std::string::npos)
        {
            parseUntilSymbol('('); //Parse it normally until the first parameter, no need to compile anything
            expressionListStarts = true;
        }
        else
            reportError("No expression list found after .\n", false);
        if (expressionListStarts)
        {
            output(tokenType);
            inExpressionList = true;
            compileExpressionList(); //Compile the parameter list
            inExpressionList = false;
        }
    }
    out << "</term>\n";
    out.flush();
}

/* Compiles an expression or term based on the token, the parameter is set to true if the method calling it is calling it
   for the second time, if not it is set to false */
bool CompilationEngine::compileAnotherExpressionOrTerm(bool isSecondCall) 
{
    char startExpressionSymbols[2] {'(', '['};
    char unaryOpSymbols[2] {'-', '~'};
    for (unsigned short i {}; i < 2; i++) 
    {
        if (tokenizer.token[0] == startExpressionSymbols[i]) //If it finds another expression then call compile expression
        {
            //No need to output the current token since compileTerm already does that
            if (isSecondCall) //If it the second call then the token is updated, so outputting it is necessary
                output(tokenType); 
            updateToken(); 
            compileExpression();

            //Since compile expression will output the same token, we dont want the program to output the same token twice
            if (!(expressionCallCount >= 1)) 
                output(tokenType);
            return true;
        }
        else if (tokenizer.token[0] == unaryOpSymbols[i]) //If it finds an unary operator then compile another term
        {
            //This makes sure if the number is negative and not subtraction, since negative numbers must have a ( placed before the -
            if (unaryOpSymbols[i] == unaryOpSymbols[0]) 
            {
                if (tokenizer.prevToken == "(")
                {
                    updateToken();
                    compileTerm();
                    return true;
                }
            }
            else
            {
                updateToken();
                compileTerm();
                return true;
            }
        }
    }
    return false; 
}



/* End of methods */

//Analyzes syntax and makes sure it matches up with the expected output parameter
bool CompilationEngine::syntaxAnalyzer(const char* expectedOutput)
{
    if (tokenizer.token != expectedOutput)
    {
        reportError(expectedOutput, true); //Report a default error
        return false;
    }
    else //If no error is found, outputs it to the xml fil
    {
        output(tokenType);
        return true;
    }
    return true;
}

//Reports an error with the expected token that was supposed to be placed, if "isDefault" is set to false it will display a custom error message
void CompilationEngine::reportError(const char* tokenOrMessage, bool isDefault)
{
    errorsFound++;
    errorHappened = true;
    if (isDefault)
        printf(RED "\nERROR:" RESET " At line %d: Expected \"%s\" after \"%s\"\n", tokenizer.lineNum, tokenOrMessage, tokenizer.prevToken.c_str());
    else
        printf(RED "\nERROR:" RESET " At line %d: %s\n", tokenizer.lineNum, tokenOrMessage);
    
    tokenizer.advance(); //Advance the token after reporting the error
    updateToken();
}