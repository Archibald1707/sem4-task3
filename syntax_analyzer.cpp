#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

enum class TokenType
{
    NULL_TOKEN = 0, // 0

    INT, STRING, BOOL,
    GOTO, LABEL,
    IF, ELSE,
    READ, WRITE,
    WHILE,
    AND, OR, NOT,
    START, FINAL, //15

    SEMICOLON, COLON, POINT, COMMA,
    ASSIGN,
    EQUAL, LESS, GREATER, NEQ, LEQ, GEQ,
    LEFT_BRACKET, RIGHT_BRACKET,
    BEGIN, END, //30

    PLUS, MINUS, MULTIPLY, DIVIDE,

    NUMBER,     //35
    IDENTIFIER, //36
    STRING_CONST, //37
    POLIZ_LABEL,
    POLIZ_ADDRESS,
    POLIZ_GO,
    POLIZ_FGO
};

class Identifier
{
private:
    std::string m_name;
    TokenType m_tokenType;

    bool m_declare;
    bool m_assign;

    int m_value;

    static int s_idCount;
    int m_idNumber;

public:
    Identifier()
            :m_assign(false), m_declare(false)
    {
    }

    Identifier(const std::string& a_name)
            :m_name(a_name), m_assign(false), m_declare(false)
    {
        m_idNumber = s_idCount;
        ++s_idCount;
    }

    bool operator==(const std::string& a_string)
    {
        return m_name == a_string;
    }

    int getID() const
    {
        return m_idNumber;
    }

    std::string getName() const
    {
        return m_name;
    }

    bool isDeclared() const
    {
        return m_declare;
    }

    void putDeclare()
    {
        m_declare = true;
    }

    TokenType getType() const
    {
        return m_tokenType;
    }

    void putType(TokenType a_tokenType)
    {
        m_tokenType = a_tokenType;
    }

    bool isAssigned() const
    {
        return m_assign;
    }

    void putAssign()
    {
        m_assign = true;
    }

    int getValue() const
    {
        return m_value;
    }

    void putValue(int a_value)
    {
        m_value = a_value;
    }
};

int Identifier::s_idCount{};

class Token
{
private:
    TokenType m_tokenType;
    int m_value;
public:
    Token(TokenType a_tokenType = TokenType::NULL_TOKEN, int a_value = 0)
    :m_tokenType(a_tokenType), m_value(a_value)
    {
    }

    TokenType getType() const
    {
        return m_tokenType;
    }

    int getValue() const
    {
        return m_value;
    }

    friend std::ostream& operator<<(std::ostream &a_output, const Token& a_token)
    {
        return a_output << '(' << (int)a_token.m_tokenType << ',' << a_token.m_value << ");";
    }
};

std::vector <Identifier> IdentifierTable;

std::vector <std::string> StringTable;

int addToIT(const std::string &buffer)
{
    auto iteratorID = std::find(IdentifierTable.begin(), IdentifierTable.end(), buffer);

    if(iteratorID != IdentifierTable.end())
        return iteratorID - IdentifierTable.begin();

    IdentifierTable.push_back(Identifier(buffer));
    return IdentifierTable.size() - 1;
}

class Scanner
{
private:
    std::ifstream m_file;
    char m_character;

    int tableLookup(const std::string& a_buffer, const std::string *a_table, int a_size)
    {
        for (int i = 0; i < a_size; ++i)
        {
            if (a_buffer == a_table[i])
                return i;
        }
        return 0;
    }

    void addBuffer(std::string &a_buffer)
    {
        a_buffer += m_character;
    }

    void getCharacter()
    {
        m_file.get(m_character);
    }

public:
    ~Scanner()
    {
        m_file.close();
    }

    static std::string serviceWordTable[], delimiterTable[];

    Scanner(const char *a_program)
    {
        m_file.open(a_program);
        if (!m_file.is_open())
        {
            throw std::runtime_error("[Scanner]: can't open file!");
        }
    }

    Token getToken();
};

std::string Scanner::serviceWordTable[] = {"not_indexed", "int", "string", "bool", "goto", "label", "if", "else",
                                           "read", "write", "while", "and", "or", "not", "program"};

std::string Scanner::delimiterTable[] = {"@", ";", ":", ".", ",", "=", "==", "<", ">", "!=", "<=",
                                         ">=", "(", ")", "{", "}", "+", "-", "*", "/"};

const int serviceWordAmount = 15, delimiterAmount = 20;

Token Scanner::getToken()
{
    enum class State
    {
        INITIALIZATION,
        IDENTIFIER,
        NUMBER,
        STRING,
        COMMENT,
        LESS_GREATER,
        NOT_EQUAL,
        DELIMITER
    };
    static int currentLine{1};
    State currentState = State::INITIALIZATION;
    std::string symbolBuffer{};
    int numberBuffer{}, inTableNumber{};
    for(;;)
    {
		int isComment = (m_character == '@');
		int isAssign = (m_character == '=');
		int isString = (m_character == '"');
        getCharacter();
        switch (currentState)
        {
            case State::INITIALIZATION:
                symbolBuffer = std::string("");
                numberBuffer = 0, inTableNumber = 0;

                if (m_file.eof())
                {
                    return Token(TokenType::FINAL);
                }

                else if (std::isspace(m_character))
                {
                    currentLine += (m_character == '\n');
                }
                else if (isalpha(m_character))
                {
                    symbolBuffer.push_back(m_character);
                    currentState = State::IDENTIFIER;
                }
                else if (isdigit(m_character))
                {
                    numberBuffer = m_character - '0';
                    currentState = State::NUMBER;
                }
                else if (isComment)
                {
                    currentState = State::COMMENT;
                }
                else if (isString)
                {
                    currentState = State::STRING;
                }
                else if (m_character == ':' || m_character == '<' || m_character == '>')
                {
                    symbolBuffer.push_back(m_character);
                    currentState = State::LESS_GREATER;
                }
                else if (m_character == '!')
                {
                    symbolBuffer.push_back(m_character);
                    currentState = State::NOT_EQUAL;
                }
                else
                {
                    symbolBuffer.push_back(m_character);
                    if ((inTableNumber = tableLookup(symbolBuffer, delimiterTable, delimiterAmount)))
                        return Token((TokenType) (inTableNumber + (int) TokenType::FINAL), inTableNumber);
                    else
                        throw m_character;
                }
                break;
            case State::IDENTIFIER:
                if (isalpha(m_character) || isdigit(m_character))
                {
                    symbolBuffer.push_back(m_character);
                }
                else
                {
                    m_file.unget();
                    if ((inTableNumber = tableLookup(symbolBuffer, serviceWordTable, serviceWordAmount)))
                    {
                        return Token((TokenType) (inTableNumber), inTableNumber);
                    }
                    else
                    {
                        inTableNumber = addToIT(symbolBuffer);
                        return Token(TokenType::IDENTIFIER, inTableNumber);
                    }
                }
                break;
            case State::NUMBER:
                if (isdigit(m_character))
                {
                    numberBuffer = numberBuffer * 10 + (m_character - '0');
                }
                else
                {
                    m_file.unget();
                    return Token(TokenType::NUMBER, numberBuffer);
                }
                break;
            case State::COMMENT:
                if (isComment)
                {
                    currentState = State::INITIALIZATION;
                }
                else if (m_file.eof())
                {
                    throw std::runtime_error("[Scanner::getToken]: eof in the middle of comment!");
                }
                else
                {
                    currentLine += (m_character == '\n');
                }
                break;
            case State::STRING:
                if (isString)
                {
                    StringTable.push_back(symbolBuffer);
                    currentState = State::INITIALIZATION;
                    return Token(TokenType::STRING_CONST, StringTable.size() - 1);
                }
                else
                {
                    symbolBuffer.push_back(m_character);
                }
            case State::LESS_GREATER:
                if (isAssign)
                {
                    symbolBuffer.push_back(m_character);
                    inTableNumber = tableLookup(symbolBuffer, delimiterTable, delimiterAmount);
                    return Token((TokenType) (inTableNumber + (int) TokenType::FINAL), inTableNumber);
                }
                else
                {
                    m_file.unget();
                    inTableNumber = tableLookup(symbolBuffer, delimiterTable, delimiterAmount);
                    return Token((TokenType) (inTableNumber + (int) TokenType::FINAL), inTableNumber);
                }
                break;
            case State::NOT_EQUAL:
                if (isAssign)
                {
                    symbolBuffer.push_back(m_character);
                    inTableNumber = tableLookup(symbolBuffer, delimiterTable, delimiterAmount);
                    return Token(TokenType::NEQ, inTableNumber);
                }
                else
                {
                    throw std::runtime_error("[Scanner::getToken]: unexpected symbol '!'.");
                }
                break;
        }
    }
}

class Parser
{
private:
    Token m_token;
    TokenType m_tokenType;
    int m_value;
    Scanner m_scanner;

    void getToken()
    {
        m_token = m_scanner.getToken();
        m_tokenType = m_token.getType();
        m_value = m_token.getValue();
    }

    void checkToken(TokenType a_expectedType)
    {
        if(m_tokenType != a_expectedType)
        {
            throw std::runtime_error("[Parser]: invalid type of token!");
        }
    }

    void getToken(TokenType a_expectedType)
    {
        getToken();
        checkToken(a_expectedType);
    }

    void value()
    {
        if (m_tokenType == TokenType::IDENTIFIER)
        {
            getToken();
        }
        else
        {
            constant();
            getToken();
        }
    }

    void multiplierOperand()
    {
        while (m_tokenType == TokenType::NOT)
        {
            getToken();
        }

        while (m_tokenType == TokenType::PLUS || m_tokenType == TokenType::MINUS)
        {
            getToken();
        }

        if (m_tokenType == TokenType::LEFT_BRACKET)
        {
            getToken();
            expression();

            checkToken(TokenType::RIGHT_BRACKET);

            getToken();
        }
        else
        {
            value();
        }

    }

    void termOperand()
    {
        multiplierOperand();

        while (m_tokenType == TokenType::MULTIPLY || m_tokenType == TokenType::DIVIDE)
        {
            getToken();
            multiplierOperand();
        }
    }

    void compareOperand()
    {
        termOperand();

        while (m_tokenType == TokenType::PLUS || m_tokenType == TokenType::MINUS)
        {
            getToken();
            termOperand();
        }
    }

    void andOperand()
    {
        compareOperand();

        bool isCompareOperand = m_tokenType >= TokenType::EQUAL &&
                                m_tokenType <= TokenType::GEQ;

        if (isCompareOperand)
        {
            getToken();
            compareOperand();
        }
    }

    void orOperand()
    {
        andOperand();

        while (m_tokenType == TokenType::AND)
        {
            getToken();
            andOperand();
        }
    }

    void assignOperand()
    {
        orOperand();

        while (m_tokenType == TokenType::OR)
        {
            getToken();
            orOperand();
        }
    }

    void expression()
    {
        assignOperand();

        while (m_tokenType == TokenType::ASSIGN)
        {
            getToken();
            assignOperand();
        }
    }

    void statement()
    {
        if (m_tokenType == TokenType::READ)
        {
            getToken(TokenType::LEFT_BRACKET);
            {
                getToken(TokenType::IDENTIFIER);
            }
            getToken(TokenType::RIGHT_BRACKET);

            getToken(TokenType::SEMICOLON);
        }
        else if (m_tokenType == TokenType::WRITE)
        {
            getToken(TokenType::LEFT_BRACKET);

            do
            {
                getToken();
                expression();
            }
            while (m_tokenType == TokenType::COMMA);

            checkToken(TokenType::RIGHT_BRACKET);

            getToken(TokenType::SEMICOLON);
        }
        else if (m_tokenType == TokenType::WHILE)
        {
            getToken(TokenType::LEFT_BRACKET);
            {
                getToken();
                expression();
            }
            checkToken(TokenType::RIGHT_BRACKET);
            {
                getToken();
                statement();
            }
        }
        else if (m_tokenType == TokenType::IF)
        {
            getToken(TokenType::LEFT_BRACKET);
            {
                getToken();
                expression();
            }
            checkToken(TokenType::RIGHT_BRACKET);
            {
                getToken();
                statement();
            }
            getToken(TokenType::ELSE);
            {
                getToken();
                statement();
            }
        }
        else if (m_tokenType == TokenType::LABEL)
        {
            getToken(TokenType::COLON);
        }
        else if (m_tokenType == TokenType::GOTO)
        {
            getToken(TokenType::LABEL);

            getToken(TokenType::SEMICOLON);
        }
        else if (m_tokenType == TokenType::BEGIN)
        {
            getToken();
            statements();
        }
        else
        {
            expression();
            checkToken(TokenType::SEMICOLON);
        }
    }

    void statements()
    {
        while (m_tokenType != TokenType::END)
        {
            statement();
            getToken();
        }
    }

    void constant()
    {
        int isAddSubtraction = m_tokenType == TokenType::MINUS
                            || m_tokenType == TokenType::PLUS;

        int isConst = m_tokenType == TokenType::NUMBER
                    && m_tokenType == TokenType::STRING_CONST;

        if (isAddSubtraction)
        {
            getToken();

            if (m_tokenType != TokenType::NUMBER)
            {
                throw std::runtime_error("[Parser]: expected number!");
            }
        }
        else if (!isConst)
        {
            throw std::runtime_error("[Parser]: expected constant!");
        }
    }

    void declarations()
    {
        auto isDeclaration = m_tokenType == TokenType::INT
                            || m_tokenType == TokenType::STRING;

        while (isDeclaration)
        {
            do
            {
                getToken(TokenType::IDENTIFIER);

                getToken();

                if (m_tokenType == TokenType::ASSIGN)
                {
                    getToken();
                    constant();
                    getToken();
                }
            }
            while (m_tokenType == TokenType::COMMA);

            checkToken(TokenType::SEMICOLON);
            getToken();
        }
    }
public:
    Parser(const char *a_file)
            :m_scanner(a_file)
    {
    }

    void analyze()
    {
        getToken(TokenType::START);
        getToken(TokenType::BEGIN);
        {
            getToken();
            declarations();
            statements();
        }
        getToken(TokenType::END);
        getToken(TokenType::FINAL);
    }
};

int main()
{
    Token token{};
    try
    {
        Scanner scanner(R"(file.txt)");
        while (token.getType() != TokenType::FINAL)
        {
            token = scanner.getToken();
            std::cout << token << '\n';
        }
        std::cout << "End of program.";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}