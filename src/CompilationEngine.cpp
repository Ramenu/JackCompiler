#include "CompilationEngine.h"
#define RESET   "\033[0m"
#define RED     "\033[31m" 

//Class constructor
CompilationEngine::CompilationEngine(const char* inputFile, const char* outputFile, const char* inputFileName)
: 
tokenizer {inputFile}, vm {outputFile}, reader {inputFile}, nArgs {}, noOfTerms {}, noifsCompiled {}, nowhilesCompiled {}, inLet {}
{
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
    vm.isNeg = false;
    reader.firstPass(); //Reads the entire file and stores the number of local variables in each subroutine into a hashmap

    tokenizer.advance(); //Advance and get the first line of the file
    updateToken(); //Get the first token, should be "class"!
    compileClass();
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

//Advances to the next line if no tokens are found
void CompilationEngine::advanceIfNoTokens()
{
    if (!tokenizer.hasMoreTokens())
    {
        tokenizer.advance();
        updateToken(); //Update the token after advancing (must be done always whenever after advancing!)
    }
}

//Updates the token
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

//Since the standard Jack classes are not included in the table, this method checks if the token is a standard Jack class or not (useful for identifying the type)
bool CompilationEngine::isStandardClass(std::string token)
{
    for (unsigned short i {}; i < JackTokenizer::numberOfClasses; i++)
        if (token == JackTokenizer::classDeclarations[i])
            return true;
    return false;
}

//Continues to parse and compile the line until a symbol is found, if append is true it will append the token to the callTo string
void CompilationEngine::parseUntilSymbol(char symbol, bool append)
{
    bool firstLoop {true};
    while (tokenizer.token[0] != symbol) //While the current token does not match the symbol
    {
        updateToken(); //Update the token
        if (append)
        {
            if (tokenizer.token[0] != symbol)
            {
                //If its the first loop iteration get the TYPE of the callee (and make sure an object/class is being accessed before getting the type)
                if (firstLoop && tokenizer.lineLeftToParse.find(".") != std::string::npos) 
                {
                    //If not a static call..
                    if (!isStandardClass(tokenizer.token))
                    {
                        callTo += table.typeOf(tokenizer.token);//Table can now find it if declared
                        vm.writePush(table.kindOf(tokenizer.token), table.indexOf(tokenizer.token)); //Push the address of the object
                        nArgs++; //Increment the number of args (since the object counts as one)
                    }
                    else
                        callTo += tokenizer.token;
                }
                else
                    callTo += tokenizer.token;
            }
            firstLoop = false;
        }
        //output(tokenType);
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
    //out << "<statements>\n";
    while (tokenIsStatement && !subroutineStarts() && !subroutineEnds())
    {
        selectCompilationTask(); 
    }
    //out << "</statements>\n";
    tokenIsStatement = false;
}

void CompilationEngine::compileIf()
{
    std::string falseLabel, trueLabel, stopLabel {};
    if (inSubroutine)
    {
        //Increment each time a if statement is compiled, to keep each if-statement in the subroutine unique
        falseLabel = "IF_FALSE" + std::to_string(noifsCompiled);
        trueLabel = "IF_TRUE" + std::to_string(noifsCompiled);
        stopLabel = "IF_END" + std::to_string(noifsCompiled); //For else statement <--
        noifsCompiled++;
    }
    tokenIsStatement = true;
    //out << "<ifStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    bool startsExpression {syntaxAnalyzer("(")};
    if (startsExpression)
    {
        updateToken();
        compileExpression(); //Compiles the expression in the if statement
        bool endsExpression {syntaxAnalyzer(")")};
        if (endsExpression)
        {
            //Should be "{", very important to update it here, because compileStatements() will not be called if not done!
            if (tokenizer.hasMoreTokens())
                updateToken(); 
            else
                advanceIfNoTokens();
            bool bodyStarts {syntaxAnalyzer("{")};
            vm.writeIf(trueLabel); //Writes an if-goto command for the "if_true" label
            vm.writeGoto(falseLabel); //Writes a goto command for the "if_false" label
            if (bodyStarts)
            {
                vm.writeLabel(trueLabel); //Writes the label for condition is true
                tokenIsStatement = false;
                if (tokenizer.hasMoreTokens())
                    updateToken(); //Even though the if ends here, if body is on the same line update it
                else //Otherwise just advance to the body of the if statement
                {
                    tokenizer.advance();
                    updateToken();
                }

                //if (noStatementMatches() == 5) //If the token is not any of the statements, output an empty statements body
                    //out << "<statements>\n</statements>\n";
                while (tokenizer.token != "}")
                    selectCompilationTask();

                syntaxAnalyzer("}"); //Make sure the if statement ends with a brace "}"
                tokenIsStatement = true;
                advanceIfNoTokens();
                bool hasElse {tokenizer.token == "else"};
                if (hasElse) //If the updated token contains an else statement, then do not end the if statement here
                {
                    vm.writeGoto(stopLabel); //Goto "STOP" since we do not want the if statement to trail into the else body
                    vm.writeLabel(falseLabel);
                    //out << "<keyword> " << tokenizer.token << " </keyword>\n"; // <-- else
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
                        //if (noStatementMatches() == 5)
                            //out << "<statements>\n</statements>\n";
                        while (tokenizer.token != "}")
                            selectCompilationTask(); // <-- body of else statement
                        syntaxAnalyzer("}"); //Should end with a "}"
                        vm.writeLabel(stopLabel);
                    }
                    tokenIsStatement = true;
                    advanceIfNoTokens();
                }
                if (!hasElse) //If there is no else statement write the label after the if statement ends
                    vm.writeLabel(falseLabel); 
            }
        }
    }
    //out << "</ifStatement>\n";    
}

void CompilationEngine::compileWhile()
{
    std::string whileLabel, endLoopLabel {};
    tokenIsStatement = true;
    if (inSubroutine)
    {
        whileLabel = "WHILE_EXP" + std::to_string(nowhilesCompiled); 
        endLoopLabel = "WHILE_END" + std::to_string(nowhilesCompiled);
        //Each time a while statement is compiled, the # of while statement compiled must increment so each while statement in the subroutine is unique 
        nowhilesCompiled++;
    }
    /* Writes the label for while statement true condition, should be put here since the VM must evaluate the expression
       each time the loop restarts */
    vm.writeLabel(whileLabel); 
    //out << "<whileStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    bool startsExpression {syntaxAnalyzer("(")};
    if (startsExpression)
    {
        updateToken();
        compileExpression();
        bool expressionEnds {syntaxAnalyzer(")")};
        vm.writeArithmetic('~'); //Write a not command (if the condition is not true)
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
                vm.writeIf(endLoopLabel); //Writes an if-goto command for the (END_LOOP) label
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
                vm.writeGoto(whileLabel); //Restart the loop and evaluate the expression again
                syntaxAnalyzer("}"); //Make sure the while statement ends with a brace "}"

                vm.writeLabel(endLoopLabel); //Writes the label for the while statement end
            }
        }
    }
    tokenIsStatement = true;
    advanceIfNoTokens();
    //out << "</whileStatement>\n";
}

void CompilationEngine::compileDo()
{
    nArgs = 0; //Initialize the number of expressions to 0
    bool hasExpressionList {};
    tokenIsStatement = true;
    //out << "<doStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    if (tokenizer.line.find("(") != std::string::npos)
    {
        hasExpressionList = true;
        callTo.clear(); //Clear the name of the previous (if any) function called before
        parseUntilSymbol('(', true);
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
            //If there is no '.' then the the name of the class must be appended (assuming the function is in THIS class)
            if (callTo.find(".") == std::string::npos) 
            {
                nArgs++; 
                callTo = className + "." + callTo;
                vm.writePush("pointer", 0); //Push the address of the caller (actually IDK what this does..)
            }
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
    //If there is no '.' then the the name of the class must be appended (assuming the function is in THIS class)
    /*if (callTo.find(".") == std::string::npos) 
    {
        nArgs++; 
        callTo = className + "." + callTo;
        vm.writePush("pointer", 0); //Push the address of the caller (actually IDK what this does..)
    }*/
    vm.writeCall(callTo, nArgs);
    vm.writePop("temp", 0); //The caller must pop/ignore the returned value
    //out << "</doStatement>\n";
    nArgs = 0;
}

void CompilationEngine::compileReturn()
{
    tokenIsStatement = true;
    //out << "<returnStatement>\n<keyword> " << tokenizer.token << " </keyword>\n";
    updateToken();
    if (tokenType == "IDENTIFIER" || tokenType == "STR_CONST" || tokenType == "INT_CONST" || tokenType == "KEYWORD")
        compileExpression();
    
    bool returnEnded {syntaxAnalyzer(";")}; //Output ";"
    //out << "</returnStatement>\n"; 
    if (returnEnded)
    {
        if (tokenizer.hasMoreTokens())
            updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
        else
            advanceIfNoTokens();
    }
    //Writes VM return
    if (subroutineType == "void") //Even a void subroutine must return something
        vm.writePush("constant", 0);
    vm.writeReturn();
}

void CompilationEngine::compileLet()
{
    bool accessArray {}, arrayExpEnds {};
    tokenIsStatement = true;
    //out << "<letStatement>\n<keyword> " << tokenizer.token << " </keyword>\n" ;
    updateToken();
    //output(tokenType); //Output the identifier
    std::string identifierName {tokenizer.token};
    updateToken();
    if (tokenizer.token == "[") //Check to see if its accessing array elements
    {
        accessArray = true;
        arrayIdentifier = identifierName;
        //output(tokenType);
        updateToken();
        compileExpression();
        arrayExpEnds = syntaxAnalyzer("]");
        updateToken();
    }
    bool hasEquals {syntaxAnalyzer("=")};
    equalToExpression = true;
    if (!accessArray || accessArray && arrayExpEnds)
    {
        if (hasEquals)
        {
            updateToken();
            inLet = true;
            compileExpression();
            inLet = false;

            //If the token is a semicolon then the let statement ends here, otherwise update to see if it is a ";"
            if (tokenizer.line.find(";") != std::string::npos)
                if (tokenizer.token != ";" && tokenizer.hasMoreTokens()) 
                    updateToken();
            bool letEnds {syntaxAnalyzer(";")};
            if (letEnds)
            {
                //out << "</letStatement>\n"; 
                if (!accessArray)
                    vm.writePop(table.kindOf(identifierName), table.indexOf(identifierName)); //Pop the value from the expression into the segment
                else
                {
                    vm.writePop("temp", 0); //Store the value from the expression in a temporary memory location
                    vm.writePop("pointer", 1); //Pop the array address into the "THAT" segment
                    vm.writePush("temp", 0);
                    vm.writePop("that", 0); //If a value is being stored in an array, the value must be put in the base address of the array index
                }
                if (tokenizer.hasMoreTokens())
                    updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
                else
                    advanceIfNoTokens();
                
            }
        }
    }
    equalToExpression = false;
}
/* End of methods */

/* Methods that compile function/method declarations, class declarations, field, static, and variable declarations */

void CompilationEngine::compileClass()
{
    tokenIsStatement = false;
    //out << "<class>\n";
    syntaxAnalyzer("class");
    updateToken(); //Get the identifier
    className = tokenizer.token;
    //output(tokenType); 
    if (tokenizer.line.find("{") != std::string::npos) //If it finds the class body declaration then it is safe to update the token
        updateToken();
    else //Otherwise it may be on the next line
    {
        tokenizer.advance();
        updateToken();
    }
    if (tokenizer.token == "{") //Check if class body is on the next line
    {
        //output(tokenType);
        tokenizer.advance();
        updateToken();
    }
    compileFile();
    syntaxAnalyzer("}"); //"}" is expected to end the class
    //out << "</class>\n";
}

void CompilationEngine::compileClassVarDec()
{
    bool endsAfterComma {};
    tokenIsStatement = false;
    if (tokenizer.line.find(";") != std::string::npos)
    {
        //out << "<classVarDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
        std::string identifierKind {tokenizer.token}; //Get the kind of the identifier (either "static" OR "field")
        updateToken();
        //output(tokenType);
        std::string identifierType {tokenizer.token}; //Get the identifier type (int, string, etc..)
        updateToken();
        //output(tokenType);
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
            //output(tokenType); //First output the identifier
            table.define(tokenizer.token, identifierType, identifierKind); //Define the identifier in the class table
            updateToken(); //Get "," or ";"
        }
        if (!endsAfterComma)
        {
            //output(tokenType); //Output ";"
            //out << "</classVarDec>\n";
            if (tokenizer.hasMoreTokens())
                updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
            else
                advanceIfNoTokens();
        }
    }
    else
        reportError("class variable declaration does not end with a \";\"", false);
}

void CompilationEngine::compileSubroutineDec()
{
    //Reset the number of ifs/while statements compiled since a new subroutine has begun
    noifsCompiled = 0;
    nowhilesCompiled = 0;

    if (tokenizer.line.find("moveBall") != std::string::npos)
    {
        printf("feorimfo");
    }
    subroutineDec = tokenizer.token; //Get the subroutine declaration type (constructor, function, method)
    table.startSubroutine(); //Clears the sub routine tables contents
    size_t noLocals {reader.getLocals(tokenizer.line)}; //Returns the number of local variables in the current subroutine
    updateToken(); //Get the identifier type
    subroutineType = tokenizer.token;
    updateToken(); //Get the identifier name
    subroutineName = tokenizer.token;
    tokenIsStatement = false;
    //out << "<subroutineDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    bool subroutineHasPara {};
    std::string classNameStr {className};
    subroutineName = classNameStr.append("." + subroutineName);
    vm.writeFunction(subroutineName.c_str(), noLocals); //Writes the VM function command
    
    if (subroutineDec == "constructor")
    {
        vm.writePush("constant", reader.numFields); 
        vm.writeCall("Memory.alloc", 1); //Allocate memory space for the object
        vm.writePop("pointer", 0); //Pop the address of the object into the "THIS" segment
    }
    else if (subroutineDec == "method")
    {
        table.noOfArgVars = 1; //Argument 0 = base address of object
        vm.writePush("argument", 0); //Argument 0 = address of the object
        vm.writePop("pointer", 0); //Put it into the "THIS" segment
    }

    if (tokenizer.line.find("(") != std::string::npos)
    {
        subroutineHasPara = true;
        parseUntilSymbol('(', false);
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
        //out << "</subroutineDec>\n"; //No advance here because subroutine body is called which has to have a { as a token
    }
    subroutineName.clear(); //Subroutine ends so erase the name so it does not keep appending
}

void CompilationEngine::compileParameterList()
{
    tokenIsStatement = false;
    //out << "<parameterList>\n";
    updateToken(); //Check the first parameter, or if the parameter ends
    while (tokenizer.token != ")")
    {
        //output(tokenType); //Output the token type
        if (tokenizer.hasMoreTokens())
            updateToken();
        else
            advanceIfNoTokens(); //In case arguments are on next line..
        if (tokenizer.token == ")") //If the parameter list ends inappropriately (right before an identifier declaration) throw an error
        {
            syntaxAnalyzer("IDENTIFIER");
            break;
        }
        //output(tokenType);
        table.define(tokenizer.token, tokenizer.prevToken, "argument"); //Adds an argument variable to the subroutine table
        if (tokenizer.hasMoreTokens())
            updateToken(); 
        else
            advanceIfNoTokens();
        
        //Check if the parameter list ends, if not each parameter must be seperated by a comma
        if (tokenizer.token != ")")
        {
            if (!syntaxAnalyzer(",")) //Each variable must be seperated by a ","
                break;
            if (tokenizer.hasMoreTokens()) //Repeat..
                updateToken(); 
            else
                advanceIfNoTokens();
        }
    }
    //out << "</parameterList>\n";
    //output(tokenType); //Output the ")"
}

void CompilationEngine::compileSubroutineBody()
{
    inSubroutine = true;
    tokenIsStatement = false;
    //out << "<subroutineBody>\n";
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
        //out << "</subroutineBody>\n";
        advanceIfNoTokens();
    }
    inSubroutine = false;
}

void CompilationEngine::compileVarDec()
{
    bool improperVarDec {};
    tokenIsStatement = false;
    //out << "<varDec>\n<keyword> " << tokenizer.token << " </keyword>\n";
    if (inSubroutine) //Variables cannot be declared outside of subroutines
    {
        if (tokenizer.line.find(";") != std::string::npos) //To avoid infinite loop
        {
            updateToken(); //Get the identifier type (int, string, classname, etc..)
            std::string identifierType {tokenizer.token};
            //output(tokenType);
            updateToken(); //Get the first identifier
            std::string identifierName {tokenizer.token};
            //output(tokenType);
            table.define(identifierName, identifierType, "local");
            updateToken(); //Check if its "," or ";"
            //Since for variable declarations, identifier names can be seperated with commas, keep adding every var to the symbol table
            while (tokenizer.token != ";")
            {
                improperVarDec = !syntaxAnalyzer(","); //Make sure each variable declared is seperated by a comma
                updateToken();
                if (tokenizer.token != ";") //If there is a semicolon right after a comma, that is incorrect, punish the programmer
                {
                    //output(tokenType);
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
                //out << "</varDec>\n";
                if (tokenizer.hasMoreTokens())
                    updateToken(); //There should not be any tokens left! But if there are it is probably a "}" so just update it
                else
                    advanceIfNoTokens();
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
    char op {' '}, currentOp {' '};
    unsigned int noOfTerms {};
    expressionCallCount++; //Increment each time compile expression is called
    bool contLoop {}, wasOutputtedBefore {}, isArrayTerm {};
    char operatorSymbols[9] {'+', '-', '*', '/', '&', '|', '<', '>', '='};
    char endExpressionSymbols[4] {')', ']', ';', ','}; //Symbols that an expression can end with
    std::string arrIdent {};
    
    //out << "<expression>\n";
   
    while (true)
    {
        if (tokenizer.prevToken == "[")
            arrIdent = arrayIdentifier;
        contLoop = false; //Reinitialize contLoop to false once/if the loop is repeated again
        wasOutputtedBefore = false;
        compileTerm(); //The token is updated when the term is compiled, so no need to update it here
        noOfTerms++;
        if (noOfTerms % 2 == 0 || noOfTerms > 2) //If over 2 terms were compiled at least or two terms are in the stack output an operator
        {
            vm.writeArithmetic(currentOp);
            wasOutputtedBefore = true;
        }
        for (unsigned short i {}; i < 9; i++)
        {
            if (tokenizer.token[0] == operatorSymbols[i]) //If the token is an operator, then there is another term.. so the expression continues
            {   
                //output(tokenType); //Output an operator
                currentOp = operatorSymbols[i];
                //Check if the next token starts an expression
                if (tokenizer.lineLeftToParse[1] == '(' || tokenizer.lineLeftToParse[1] == '[') 
                    op = operatorSymbols[i]; //Store the current operator for later
                updateToken();
                contLoop = true; //Repeat and compile a term again
                break;
            }
        }
        if (contLoop)
            continue;
        for (unsigned short i {}; i < 4; i++)
        {
            if (tokenizer.token[0] == endExpressionSymbols[i]) //If the current token equals a symbol that ends an expression, end the loop
                break; 
        }
        //TODO: Output error message if there is no end expression 
        break;
    }
    //out << "</expression>\n";
    /* If there is more than one expression being compiled and the expression finishes, we want to update the token
       since the expression does not actually end yet */
    if (tokenizer.token == ")" || tokenizer.token == "]")
    { 
        if (!arrIdent.empty() && tokenizer.token == "]")
        {
            size_t startArrExpCount {};
            vm.writePush(table.kindOf(arrIdent), table.indexOf(arrIdent));
            vm.writeArithmetic('+');
            for (auto&i: tokenizer.lineLeftToParse)
                if (i == '[')
                    startArrExpCount++;
            if (expressionCallCount > 1)
            {
                vm.writePop("pointer", 1);
                vm.writePush("that", 0);
            }
            arrIdent.clear();
        }
        //If we are not compiling do, then expression calls expression list, ECP must be over or equal to 2 to be updated
        if (inExpressionList && !inDo && expressionCallCount > 2) 
        {
            if (inTerm && reader.instancesOf(tokenizer.lineLeftToParse, ')') > 1 || !inTerm) 
                updateToken();
        }
        //Since compiling do calls compile expression list ECP would have to be over 1 to be updated
        else if (inExpressionList && inDo && expressionCallCount > 1)
        {
            //output(tokenType);
            updateToken();
        }
        if (!inExpressionList && expressionCallCount > 1)
        {
            updateToken();
        }
    }
    if (!wasOutputtedBefore && currentOp != ' ') //Make sure it was not already outputted and actually contains an operator
        vm.writeArithmetic(currentOp); //Since the expression ends, output the operator that was not outputted
    //vm.writeArithmetic(currentOp);
    expressionCallCount--; //Decrement when the method finishes, should be 0 when there is expression being compiled in the call stack
}

void CompilationEngine::compileExpressionList()
{
    //out << "<expressionList>\n";
    updateToken(); //Get the first expression
    while (tokenizer.token != ")" && tokenizer.token != ";")
    {
        compileExpression();
        nArgs++; //For each expression found in the expression list, +1

        //The expression list must end with a parenthesis, and each expression, each time compiled, must be seperated with a comma
        if (tokenizer.token != ")" && tokenizer.token == ",") 
        {
            bool isComma {syntaxAnalyzer(",")};
            if (!isComma)
                break;
            updateToken();
        }
    }
    //out << "</expressionList>\n";
    //output(tokenType); //Output ")"
}

void CompilationEngine::compileTerm()
{
    bool arrayIdentifier {};
    size_t numArgsLocal {};
    std::string calleeClassName {};
    //out << "<term>\n";
    //output(tokenType);
    if (!compileAnotherExpressionOrTerm(false)) //Check if the current token starts an expression or term, if it returns false..
    {
        if (!reader.startsArray(tokenizer.lineLeftToParse)) //If the token right after the current one is not a '['
        {
            if (tokenType == "INT_CONST")
            {
                vm.writePush("constant", std::stoi(tokenizer.token));
            }
            else if (tokenType == "STRING_CONST")
            {
                compileString();
            }
            else if (tokenizer.token == "true" || tokenizer.token == "false" || tokenizer.token == "null") //If it is a boolean value
            {
                vm.writePush("constant", 0); //0 for false
                if (tokenizer.token == "true") //If it is true negate it to make it -1, which will make it true
                    vm.writeArithmetic('~'); 
            }
            //If it is an identifier and it does not call a subroutine
            else if (tokenType == "IDENTIFIER" && !isStandardClass(tokenizer.token))
            {
                calleeClassName = tokenizer.token;
                vm.writePush(table.kindOf(tokenizer.token), table.indexOf(tokenizer.token));
            }
            else if (tokenizer.token == "this") //If a reference to the object
            {
                vm.writePush("pointer", 0); //Push the address of the object
            }
        }
        updateToken(); //Update the token
        compileAnotherExpressionOrTerm(true); //Check if this token starts an expression or term
    }
    if (tokenizer.token == ".") //Check if accessing class/object
    {
        bool expressionListStarts {};
        //output(tokenType);
        if (tokenizer.line.find("(") != std::string::npos)
        {
            callTo.clear();
            //WHY IS THIS HERE..?
            if (!isStandardClass(tokenizer.prevToken)) //Check if the method is accessed by an object
            {
                callTo += table.typeOf(tokenizer.prevToken);
                //vm.writePush(table.kindOf(tokenizer.prevToken), table.indexOf(tokenizer.prevToken)); //Push the address of the object
            }
            else
                callTo = tokenizer.prevToken; //Otherwise append it normally

            updateToken(); //Get the method/function name after '.'
            callTo += "." + tokenizer.token;

            for (size_t i {}; i < tokenizer.lineLeftToParse.length(); i++)
                if (tokenizer.lineLeftToParse[i] == ',') //After a comma is always an argument/expression (if written correctly)
                    numArgsLocal++;

            //If there is more than one argument, increment the number of arguments (since there is no ',' for the first expression)
            if (tokenizer.lineLeftToParse.find("()") == std::string::npos) //If not an empty expression list
                numArgsLocal++; //Increment
            if (!calleeClassName.empty()) //Object is argument 0, so increment by 1
                numArgsLocal++; 

            expressionListStarts = true;
        }
        else
            reportError("No expression list found after .\n", false);
        if (expressionListStarts)
        {
            //output(tokenType);
            inExpressionList = true;
            inTerm = true;
            compileExpressionList(); //Compile the parameter list
            inTerm = false;
            inExpressionList = false;
            vm.writeCall(callTo, numArgsLocal); //Writes a VM call command to the subroutine being called and its # of arguments
        }
    }
    //out << "</term>\n";
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
            //if (isSecondCall) //If it the second call then the token is updated, so outputting it is necessary
                //output(tokenType); 
            if (startExpressionSymbols[i] == '[')
                arrayIdentifier = tokenizer.prevToken;
            updateToken(); 
            compileExpression();

            //Since compile expression will output the same token, we dont want the program to output the same token twice
            //if (!(expressionCallCount >= 1)) 
                //output(tokenType);
            return true;
        }
        else if (tokenizer.token[0] == unaryOpSymbols[i]) //If it finds an unary operator then compile another term
        {
            //This makes sure if the number is negative and not subtraction, since negative numbers must have a ( placed before the -
            if (unaryOpSymbols[i] == unaryOpSymbols[0]) 
            {
                if (tokenizer.prevToken == "(" || tokenizer.prevToken == "," || tokenizer.prevToken == "=")
                {
                    vm.isNeg = true;
                    updateToken();
                    compileTerm();
                    vm.writeArithmetic('-');
                    vm.isNeg = false;
                    return true;
                }
            }
            else
            {
                updateToken();
                compileTerm();
                vm.writeArithmetic('~');
                return true;
            }
        }
    }
    return false; 
}


/* End of methods */

//Not in the API, but useful for automatically compiling strings (should be called only if tokentype == "STR_CONST")
void CompilationEngine::compileString()
{
    vm.writePush("constant", tokenizer.tokenString.length()); //Push the length of the string
    vm.writeCall("String.new", 1); //Allocate memory for the string
    for (size_t i {}; i < tokenizer.tokenString.length(); i++)
    {
        vm.writePush("constant", static_cast<int>(tokenizer.tokenString[i])); //Push the ASCI # of the character
        vm.writeCall("String.appendChar", 2); //Append the character to the string
    }
    tokenizer.tokenString.clear(); //Empty the string once it is compiled
}

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
        ////output(tokenType);
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