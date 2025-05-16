#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_LENGTH 256

#define TYPE_VAR    1
#define TYPE_NUM    2
#define TYPE_ATOM   (TYPE_VAR|TYPE_NUM)
#define TYPE_OP     4

#define BP_LEFT     0
#define BP_RIGHT    1

typedef struct{
    char *start;
    size_t length;
}SubString;

typedef struct {
    int type;
    union {
        char op;
        int number;
        SubString string;
    }value;
}Token;

typedef struct {
    Token *tokens;
    size_t count;
    size_t scanner;
}Lexer;

void LexerAnalysis(Lexer *lexer, char *str)
{
    lexer->tokens = (Token*) malloc( sizeof(Token)*256 );
    lexer->count = 0;
    lexer->scanner = 0;

    int i = 0;
    while (str[i])
    {
        if ('0' <= str[i] && str[i] <= '9')
        {
            int num = 0;
            while('0' <= str[i] && str[i] <= '9')
            {
                num=num*10+str[i]-'0';
                i++;
            }
            lexer->tokens[lexer->count].type = TYPE_NUM;
            lexer->tokens[lexer->count].value.number = num;
            lexer->count++;
        }

        else if ('a' <= str[i] && str[i] <= 'z' ||
                'A' <= str[i] && str[i] <= 'Z' ||
                str[i] == '_')
        {
            int len = 0;
            while('0' <= str[i+len] && str[i+len] <= '9' ||
                'a' <= str[i+len] && str[i+len] <= 'z' ||
                'A' <= str[i+len] && str[i+len] <= 'Z' || 
                str[i+len] == '_')
            {
               len++;
            }
            lexer->tokens[lexer->count].type = TYPE_VAR;
            lexer->tokens[lexer->count].value.string.start = (str+i);
            lexer->tokens[lexer->count].value.string.length = len;
            i+=len;
            lexer->count++;
        }

        else if(str[i] != ' ' && str[i] != '\n')
        {
            lexer->tokens[lexer->count].type = TYPE_OP;
            lexer->tokens[lexer->count].value.op = str[i];
            lexer->count++;
            i++;
        }
        else i++;
    }
}

char bp[128][2];

int GetBindingPower(char op,int lr)
{
    switch (op)
    {
        case '+': return lr == BP_LEFT ? 5 : 6; break;
        case '-': return lr == BP_LEFT ? 5 : 6; break;
        case '*': return lr == BP_LEFT ? 10 : 11; break;
        case '/': return lr == BP_LEFT ? 10 : 11; break;
        case '=': return lr == BP_LEFT ? 1 : 2; break;
        default:
            puts("invalid operator");
            exit(0);
    }
}

typedef struct Expression Expression;
struct Expression
{
    Token token;
    Expression *lhs;
    Expression *rhs;
};

typedef struct
{
    char *pool;
    size_t block;
    size_t count;
    size_t index;
}MemPool;

void PoolInit(MemPool *pool, size_t block ,size_t count)
{
    pool->pool = (char *) malloc(block * count);
    pool->count = count;
    pool->block = block;
    pool->index = 0;
}

char *PoolAlloc(MemPool *pool, size_t size)
{
    if(pool->index + 1 >= pool->count * pool->block)
    {
        puts("memory exceeded");
        exit(0);
    }
    char *pmem = &pool->pool[pool->index];
    pool->index += ((size + pool->block - 1) & ~(pool->block - 1));
    return pmem;
}

void PoolDealloc(MemPool *pool)
{
    pool->index = 0;
}

void PoolDestroy(MemPool *pool)
{
    free(pool->pool);
}

Expression *SyntaxAnalysis(MemPool *pool, Lexer *lexer, int minBindingPower, int inBracket){
    if (lexer->count == 0)
    {
        return NULL;
    }
    
    Expression *lhs = (Expression*) PoolAlloc(pool, sizeof(Expression));
    if (lexer->tokens[lexer->scanner].type & TYPE_ATOM)
    {
        lhs->token = lexer->tokens[lexer->scanner];
        lhs->lhs = NULL;
        lhs->rhs = NULL;
    }
    else if (lexer->tokens[lexer->scanner].type == TYPE_OP &&
             lexer->tokens[lexer->scanner].value.op == '(')
    {
        lexer->scanner++;
        lhs = SyntaxAnalysis(pool, lexer, 0, 1);
        lexer->scanner++;
        if(lexer->tokens[lexer->scanner].type != TYPE_OP ||
            lexer->tokens[lexer->scanner].value.op != ')')
        {
            puts("missing right bracket");
            exit(0);
        }
    }
    else
    {
        puts("missing left expression");
        exit(0);
    }

    while (lexer->scanner != lexer->count - 1)
    {
        if(lexer->tokens[lexer->scanner+1].type == TYPE_OP)
        {
            if(lexer->tokens[lexer->scanner+1].value.op == ')')
            {
                if(inBracket == 1)
                {
                    break;
                }
                else
                {
                    puts("missing left bracket");
                    exit(0);
                }
                
            }
            int opIndex = lexer->scanner+1;
            if(minBindingPower < GetBindingPower(lexer->tokens[opIndex].value.op, BP_LEFT))
            {
                lexer->scanner+=2;
                if(lexer->scanner >= lexer->count)
                {
                    puts("missing right expression");
                    exit(0);
                }
                Expression *rhs = 
                    SyntaxAnalysis(pool, lexer, GetBindingPower(lexer->tokens[opIndex].value.op, BP_RIGHT), inBracket);
                Expression *expr = (Expression*) PoolAlloc(pool, sizeof(Expression));
                expr->token = lexer->tokens[opIndex];
                expr->lhs = lhs;
                expr->rhs = rhs;
                lhs = expr;
            }
            else break;
        }
        else
        {
            puts("missing operator");
            exit(0);
        }
    }
    return lhs;
}

void PrintSyntaxTree(Expression *expr, int level)
{
    if(expr == NULL)
    {
        return ;
    }
    int i = level;
    while(i--)
    {
        putchar(' ');
        putchar(' ');
    }
    switch(expr->token.type)
    {
        case TYPE_NUM:
            printf("%d\n",expr->token.value.number);
            break;
        case TYPE_OP:
            printf("%c\n",expr->token.value.op);
            break;
        case TYPE_VAR:
            i = 0;
            while(i<expr->token.value.string.length)
            {
                putchar(*(expr->token.value.string.start+i));
                i++;
            }
            putchar('\n');
            break;
    }
    PrintSyntaxTree(expr->lhs, level+1);
    PrintSyntaxTree(expr->rhs, level+1);
}

typedef struct SymbolNode SymbolNode;
struct SymbolNode
{
    int32_t hash;
    int value;
    SymbolNode *next;
};
int32_t MurmurHash2(uint32_t seed, SubString name)
{
    uint32_t hash = seed ^ name.length;

    for (int i=0; i<name.length ; i++)
    {
        uint32_t k = name.start[i];
        k *= 0x5bd1e995;
        k ^= k >> 24;
        hash *= 0x5bd1e995;
        hash ^= k;
    }

    hash ^= hash >> 13;
    hash *= 0x5bd1e995;
    hash ^= hash >> 15;
    return hash;
}
int Evaluate(Expression *expr, SymbolNode *table)
{
    if(expr->token.type & TYPE_ATOM)
    {
       if(expr->token.type == TYPE_NUM)
            return expr->token.value.number;
        else
        {
            SymbolNode *pos = table;
            while(pos != NULL && MurmurHash2(0, expr->token.value.string) != pos->hash)
            {
                pos = pos->next;
            }
            if (pos == NULL)
            {
                puts("undefined variable");
                exit(0);
            }
            return pos->value;
        }
    }
    else
    {
        int lhs = Evaluate(expr->lhs, table);
        int rhs = Evaluate(expr->rhs, table);
        switch (expr->token.value.op)
        {
            case '+': return lhs + rhs;
            case '-': return lhs - rhs;
            case '*': return lhs * rhs;
            case '/': return lhs / rhs;
            default:
                puts("invalid operator");
                exit(0);
        }
    }
}

int IsAssignment(Expression *expr)
{
    if(expr == NULL || expr->lhs == NULL || expr->rhs == NULL)
        return 0;
    if(expr->lhs->token.type == TYPE_VAR && expr->token.value.op == '=')
        return 1;
    return 0;
}


int main()
{
    char str[256];

    MemPool pool;
    PoolInit(&pool, 16, 256);

    SymbolNode *table = NULL;

    while (1)
    {
        printf(">> ");
        fgets(str, 256, stdin);

        Lexer lexer;
        LexerAnalysis(&lexer, str);

        Expression *expr = SyntaxAnalysis(&pool, &lexer, 0, 0);
        // PrintSyntaxTree(expr, 0);

        if(IsAssignment(expr))
        {
            SymbolNode *pos = table;
            SymbolNode *pre = table;
            while(pos != NULL && MurmurHash2(0, expr->lhs->token.value.string) != pos->hash)
            {
                pre = pos;
                pos = pos->next;
            }
            if(pos == NULL)
            {
                pos = (SymbolNode*) PoolAlloc(&pool, sizeof(SymbolNode));
                pos->hash = MurmurHash2(0, expr->lhs->token.value.string);
                pos->next = NULL;
                if(table == NULL)
                {
                    table = pos;
                }
                else
                {
                    pre->next = pos;
                }
            }
            pos->value = Evaluate(expr->rhs, table);
        }
        else
        {
            printf("%d\n",Evaluate(expr, table));
        }
        free(lexer.tokens);
    }
    PoolDestroy(&pool);
}
