#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Definição dos tamanhos máximos de campos e registros
#define MAX_FIELDS 10
#define MAX_RECORDS 100
#define MAX_FIELD_LENGTH 50

// Estrutura para definir um campo de tabela (nome e tipo)
typedef struct {
    char field_name[MAX_FIELD_LENGTH];  // Nome do campo
    char field_type;                    // Tipo do campo (C ou N)
} Field;

// Estrutura para definir uma tabela com campos e registros
typedef struct {
    char table_name[MAX_FIELD_LENGTH];  // Nome da tabela
    Field fields[MAX_FIELDS];           // Array de campos da tabela
    char records[MAX_RECORDS][MAX_FIELDS][MAX_FIELD_LENGTH]; // Array de registros
    int field_count;                    // Número de campos na tabela
    int record_count;                   // Número de registros na tabela
} Table;

Table tables[10];                       // Array de tabelas
int table_count = 0;                    // Contador de tabelas

void show_table(Table *table);          // Função para exibir dados de uma tabela
int execute_command(const char *input); // Função para executar comandos
Table* get_table_by_name(const char *name); // Função para buscar uma tabela por nome

// Função que exibe a tabela e seus registros
void show_table(Table *table) {
    // Imprime o nome de cada campo
    for (int i = 0; i < table->field_count; i++) {
        printf("%s\t", table->fields[i].field_name);
    }
    printf("\n");

    // Imprime cada registro
    for (int i = 0; i < table->record_count; i++) {
        for (int j = 0; j < table->field_count; j++) {
            printf("%s\t", table->records[i][j]);
        }
        printf("\n");
    }
}

// Função para buscar uma tabela pelo nome
Table* get_table_by_name(const char *name) {
    for (int i = 0; i < table_count; i++) {
        if (strcmp(tables[i].table_name, name) == 0) {
            return &tables[i];  // Retorna o ponteiro para a tabela
        }
    }
    return NULL; // Retorna NULL se a tabela não for encontrada
}

// Definição de tipos de tokens para análise léxica
typedef enum {
    TOKEN_EOF,
    TOKEN_CREATE,
    TOKEN_TABLE,
    TOKEN_INSERT,
    TOKEN_INTO,
    TOKEN_SELECT,
    TOKEN_FROM,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_COMMA,
    TOKEN_LPAREN,  // (
    TOKEN_RPAREN,  // )
    TOKEN_SEMICOLON,
    TOKEN_ASTERISK // *
} TokenType;

// Estrutura para definir um token (tipo e valor)
typedef struct {
    TokenType type;
    char value[MAX_FIELD_LENGTH];
} Token;

// Função que extrai o próximo token da entrada
Token get_token(const char **input) {
    Token token;
    token.type = TOKEN_EOF;
    token.value[0] = '\0';

    // Ignora espaços, tabulações e novas linhas
    while (**input == ' ' || **input == '\n' || **input == '\t' || **input == '\r') {
        (*input)++;
    }
    
    // Verifica palavras-chave e símbolos e atribui o tipo de token
    if (strncmp(*input, "CREATE", 6) == 0) {
        (*input) += 6;
        token.type = TOKEN_CREATE;
    } else if (strncmp(*input, "TABLE", 5) == 0) {
        (*input) += 5;
        token.type = TOKEN_TABLE;
    } else if (strncmp(*input, "INSERT", 6) == 0) {
        (*input) += 6;
        token.type = TOKEN_INSERT;
    } else if (strncmp(*input, "INTO", 4) == 0) {
        (*input) += 4;
        token.type = TOKEN_INTO;
    } else if (strncmp(*input, "SELECT", 6) == 0) {
        (*input) += 6;
        token.type = TOKEN_SELECT;
    } else if (strncmp(*input, "FROM", 4) == 0) {
        (*input) += 4;
        token.type = TOKEN_FROM;
    } else if (**input == ',') {
        (*input)++;
        token.type = TOKEN_COMMA;
    } else if (**input == '(') {
        (*input)++;
        token.type = TOKEN_LPAREN;
    } else if (**input == ')') {
        (*input)++;
        token.type = TOKEN_RPAREN;
    } else if (**input == ';') {
        (*input)++;
        token.type = TOKEN_SEMICOLON;
    } else if (**input == '*') {
        (*input)++;
        token.type = TOKEN_ASTERISK;
    } else if (isdigit(**input)) {
        int i = 0;
        while (isdigit(**input)) {
            token.value[i++] = *(*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
    } else if (isalpha(**input)) {
        int i = 0;
        while (isalnum(**input) || **input == '_') {
            token.value[i++] = *(*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_STRING;
    }
    return token;
}

// Função para validar o tipo de dado de um valor inserido
int validate_field_value(const char *value, char field_type) {
    if (field_type == 'N') { // Campo numérico
        for (int i = 0; value[i] != '\0'; i++) {
            if (!isdigit(value[i])) {
                return 0;  // valor inválido
            }
        }
    } else if (field_type == 'C') { // Campo de texto
        for (int i = 0; value[i] != '\0'; i++) {
            if (!isalpha(value[i])) {  // Aceita apenas letras
                return 0;  // valor inválido
            }
        }
    }
    return 1; // valor válido
}

// Função para processar o comando CREATE TABLE
int parse_create_table(const char **input) {
    Token token = get_token(input);
    if (token.type == TOKEN_STRING) {
        Table *table = &tables[table_count++];
        strcpy(table->table_name, token.value);
        table->field_count = 0;
        token = get_token(input);
        if (token.type == TOKEN_LPAREN) {
            int field_index = 0;
            do {
                token = get_token(input);
                if (token.type == TOKEN_STRING) {
                    strcpy(table->fields[field_index].field_name, token.value);
                    token = get_token(input);
                    if (strcmp(token.value, "C") == 0) {
                        table->fields[field_index].field_type = 'C';
                    } else if (strcmp(token.value, "N") == 0) {
                        table->fields[field_index].field_type = 'N';
                    } else {
                        printf("Erro de sintaxe: Tipo de campo inválido.\n");
                        return 1;
                    }
                    field_index++;
                    table->field_count++;
                }
                token = get_token(input);
            } while (token.type == TOKEN_COMMA);
            if (token.type != TOKEN_RPAREN) {
                printf("Erro de sintaxe: Esperado ')'.\n");
                return 1;
            }
        } else {
            printf("Erro de sintaxe: Esperado '('.\n");
            return 1;
        }
    } else {
        printf("Erro de sintaxe: Nome da tabela esperado.\n");
        return 1;
    }
    return 0;
}

// Função para processar o comando INSERT INTO, incluindo validação de tipo
int parse_insert_into(const char **input) {
    Token token = get_token(input);
    if (token.type == TOKEN_STRING) {
        Table *table = get_table_by_name(token.value);
        if (!table) {
            printf("Erro: Tabela '%s' não encontrada.\n", token.value);
            return 1;
        }
        token = get_token(input);
        if (token.type == TOKEN_LPAREN) {
            int record_index = table->record_count;
            int field_index = 0;
            do {
                token = get_token(input);
                if (token.type == TOKEN_STRING || token.type == TOKEN_NUMBER) {
                    // Valida o valor conforme o tipo do campo
                    if (field_index < table->field_count &&
                        !validate_field_value(token.value, table->fields[field_index].field_type)) {
                        printf("Erro de sintaxe: Tipo de valor inválido para o campo '%s'.\n",
                               table->fields[field_index].field_name);
                        return 1;
                    }
                    if (field_index < table->field_count) {
                        strcpy(table->records[record_index][field_index], token.value);
                        field_index++;
                    }
                }
                token = get_token(input);
            } while (token.type == TOKEN_COMMA);

            table->record_count++;
            printf("Registro inserido com sucesso! (Registro %d)\n", record_index + 1);
        } else {
            printf("Erro de sintaxe: Esperado '('.\n");
            return 1;
        }
    } else {
        printf("Erro de sintaxe: Nome da tabela esperado.\n");
        return 1;
    }
    return 0;
}

// Função para processar o comando SELECT
int parse_select_from(const char **input) {
    Token token = get_token(input);
    if (token.type == TOKEN_ASTERISK) {
        token = get_token(input);
        if (token.type == TOKEN_FROM) {
            token = get_token(input);
            if (token.type == TOKEN_STRING) {
                Table *table = get_table_by_name(token.value);
                if (table) {
                    show_table(table);
                    token = get_token(input);  // Verifica se há um ';' após SELECT
                    if (token.type != TOKEN_SEMICOLON && token.type != TOKEN_EOF) {
                        printf("Erro de sintaxe: Comando inesperado após SELECT.\n");
                        return 1;
                    }
                    return 0;
                } else {
                    printf("Erro: Tabela '%s' não encontrada.\n", token.value);
                    return 1;
                }
            } else {
                printf("Erro de sintaxe: Nome da tabela esperado após FROM.\n");
                return 1;
            }
        } else {
            printf("Erro de sintaxe: Esperado 'FROM'.\n");
            return 1;
        }
    } else {
        printf("Erro de sintaxe: Esperado '*'.\n");
        return 1;
    }
}

// Função que executa o comando baseado na entrada
int execute_command(const char *input) {
    const char *ptr = input;
    Token token = get_token(&ptr);
    if (token.type == TOKEN_CREATE) {
        token = get_token(&ptr);
        if (token.type == TOKEN_TABLE) {
            if (parse_create_table(&ptr) != 0) return 1;
            printf("Tabela criada com sucesso!\n");
        }
    } else if (token.type == TOKEN_INSERT) {
        token = get_token(&ptr);
        if (token.type == TOKEN_INTO) {
            if (parse_insert_into(&ptr) != 0) return 1;
        }
    } else if (token.type == TOKEN_SELECT) {
        if (parse_select_from(&ptr) != 0) return 1;
    } else {
        printf("Erro de sintaxe: Comando desconhecido.\n");
        return 1;
    }
    return 0;
}

// Função principal que lê o arquivo e executa os comandos
int main() {
    table_count = 0;

    char filename[100];

    printf("\nInterpretador DBASE\n");
    printf("CMP1076 - COMPILADORES\n");
    printf("Aluno: Leonardo de Moura Alves\n\n");

    printf("Informe o nome do arquivo .txt com os comandos: ");
    scanf("%99s", filename);

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove novas linhas e espaços ao final
        line[strcspn(line, "\r\n")] = 0;

        // Ignora linhas vazias
        if (strlen(line) > 0) {
            if (execute_command(line) != 0) {
                printf("Interrompendo execução devido a erro.\n");
                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);

    printf("\nFinalizado\n");
    return 0;
}
