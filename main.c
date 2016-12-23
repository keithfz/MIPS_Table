#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


//This node structure is a linked list that holds the identifiers uses.
//Each of the elements in the array of struct data points to a node if the identifier has a use
struct node{
    int line_used;
    struct node* next_use;
};

//The struct data is used to hold the identifier name and the line it was defined
//It points to a linked list of the uses of the identifier (if applicable)
struct data{
    char identifier[11];
    int line_defined;
    struct node* uses;
};

//Prototypes for functions
void programListing(FILE* infile, FILE* outfile);
void crossReferenceTable(FILE* infile, FILE* outfile,const char* line);
void addToList(char* identifier_use, int lineCount);
void addToData(char* token, int lineDef);
void printCrossTable(FILE* outfile);
int allWhiteSpace(char line[80]);
int compareToArray(char* token);

//global variables
struct node* head;
struct data data_array[100];
int i = 0; //i holds position in data_array

//===================================================================================
//main
int main(int argc, const char * argv[]) {
    //these will be the files passed in as arguments
    FILE *infile;
    FILE *outfile;
    
    //if there is a wrong number of arguments, it will print an error message to stderr
    if(argc != 4){
        perror("Wrong number of arguments.\n");
        return (-1);
    }
    
    //Sets infile and outfile to the files passed in as args
    //if the file does not open properly, it prints error message to stderr
    infile = fopen(argv[2], "r");
    if(infile == NULL){
        perror("Error opening file.\n");
        return (-1);
    }
    
   test_lines(infile); rewind(infile);
    
    
    outfile = fopen(argv[3], "w");
    if(outfile == NULL){
        perror("Error opening file.\n");
    }
    
    //if the command is -l, it will print the program listing
    if(strcmp(argv[1],"-l")==0){
        programListing(infile,outfile);
    }
    
    //if the command is -c it will print the cross reference table
    if (strcmp(argv[1], "-c")==0) {
        crossReferenceTable(infile, outfile,argv[2]); //this constructs the data structures for the crt
        printCrossTable(outfile); //this prints the crt
    }
    
    //if the command is -b, it will print both the program listing and the cross reference table
    if (strcmp(argv[1], "-b")==0) {
        programListing(infile, outfile);
        rewind(infile); //rewind is used to return the infile to the beginning from EOF
        crossReferenceTable(infile, outfile, argv[2]);
        fprintf(outfile, "\n\n");
        printCrossTable(outfile);
    }
    
    
    //close the files used
    fclose(infile);
    fclose(outfile);
}

//===================================================================================
//writes the program listing with line numbers to the output file
void programListing(FILE* infile, FILE* outfile){
    char read_line[80]; //this holds the each line of the file
    int lineCount = 0;  //this is a count that increments each non-empty line
    while(1){
        //this sets read_line to each line in the text file until its null
        //if it is null, it exits the loop
        if(fgets(read_line, 81, infile)==NULL) break;
        
        //if the line is entirely whitespace, then echo the empty line to the output
        if(allWhiteSpace(read_line)!=0){
            fprintf(outfile, "\n");
        }
        //if the line is not entirely whitespace, print the linecount followed by the line
        else{
            lineCount++;
            fprintf(outfile, "%d\t%s",lineCount,read_line);
        }
    }
}





//This function constructs the data structures used for the cross reference table
void crossReferenceTable(FILE* infile, FILE* outfile,const char* lineName){
    int lineCount = 0;  //lineCount counts the number of lines
    int temp;
    int colonCount;     //colonCount counts the number of times a colon was used in the line of text
    char read_line[80]; //read_line is the line of text taken from the infile with fgets
    
    //these are the tokens of read_line that use whitespace as the delimiter
    char* token1_ptr = NULL;
    char* token2_ptr = NULL;
    char* token3_ptr = NULL;

    char* saveptr; //saveptr is the reentrant pointer saved when using strtok_r
    char* identifier_ptr = NULL; //identifier_ptr is the token of the identifier we want to find
    char* identifier_ptr2 = NULL;
    char* use_ptr = NULL; //use_ptr is the token of the uses of the identifier
    FILE* infile2 = fopen(lineName, "r"); //infile2 is a second opening of the infile
    char read_line2[80]; //read_line2 is a second read_line used for the second while loop
    
    //This loop loops through the text file and looks for the declaration of the identifiers
    while(1){
        colonCount = 0;
        if(fgets(read_line, 81, infile)==NULL) break; //exits the loop if it reaches EOF
        
        
        temp = 0;
        int length = strlen(read_line);
        
    
        //this gets the number of colons in the line
        while (temp<length) {
            if (read_line[temp] == ':')
                    colonCount++;
            temp++;
        }
        
        //if the line is not all whitespace, then increment the lineCount
        if (allWhiteSpace(read_line) == 0) {
            lineCount++;
        }
        
        if (read_line[0]!='#') {

        
        //if there is a colon in the line then take its token and add it to the array
        if(colonCount > 0){
            identifier_ptr = strtok(read_line," \t");
            identifier_ptr2 = strtok(identifier_ptr, ":\t ");
            addToData(identifier_ptr2, lineCount);
        
        }
        }
        temp = 0;
        while(temp<80){
            read_line2[temp] = '\0';
            temp++;
        }
    }//end of first while loop
    
    //resets the lineCount
    lineCount = 0;
    
    //this while loop looks for the uses of the declared identifiers and adds them to a linked list
    while(1){
        if (fgets(read_line2, 81, infile2)==NULL) break; //exits look if reached EOF
        
        //if the line is not entirely whitespace, increment the lineCount
        if (allWhiteSpace(read_line2) == 0) {
            lineCount++;
        }
        
        //if it is not a comment line, contine
        if(read_line2[0] != '#'){
            colonCount = 0;
            temp = 0;
            int length = strlen(read_line2);
        
            //gets the number of colons used in read_line2
            while (temp<length) {
                if (read_line2[temp] == ':')
                    colonCount++;
            temp++;
            }
            

            
            //this breaks the line up into at most 3 tokens
            token1_ptr = strtok_r(read_line2," \t\r\n\v\f",&saveptr);
            if(saveptr!=NULL){
                token2_ptr = strtok_r(NULL," \t\r\n\v\f",&saveptr);
            }
            if(saveptr!=NULL){
                token3_ptr = strtok_r(NULL,"\t\r\n#",&saveptr);
            }


        
            //this is the case where token3 is not null
            if(token3_ptr!=NULL){
                //tokenize token 3 to extract possible identifiers used
                use_ptr = strtok(token3_ptr, ", \t");
                //compare these tokens to the identifiers stored in the array
                while (use_ptr != NULL) {
                    //if the token is a match to an identifier, add the use to a linked list
                    if(compareToArray(use_ptr)==0){
                        addToList(use_ptr, lineCount);
                        use_ptr = strtok(NULL, ", \t");
                    }
                }
            }
            
            //this is the case where there is no identifier declared and token 2 is not null
            if(colonCount == 0 && token2_ptr!=NULL){
                //tokenize token 2 to extract possible identifiers used
                use_ptr = strtok(token2_ptr, ", \t\n\r#");
                while (use_ptr != NULL) {
                    //compare these tokens to the identifiers stored in the array
                    if(compareToArray(use_ptr)==0){
                        //if the token is a match to an identfier, add the use to a linked list
                        addToList(use_ptr, lineCount);
                        use_ptr = strtok(NULL, ", \t\n\r#");
                    }
                }
            }
        }
        
        //this resets the counts and tokens
        temp = 0;
        while(temp<80){
            read_line2[temp] = '\0';
            temp++;
        }
        token1_ptr = NULL;
        token2_ptr = NULL;
        token3_ptr = NULL;
    }//end of second while loop
    
    
}

//the addToList function adds nodes to a linked list of identifier uses
void addToList(char* identifier_use, int lineCount){
    int j = 0;
    struct node* current;
    
    //while the temp variable j is less than the number of elements in data_array
    while(j<i){
        //if the identifier of the token is a match to the identifier in the array
        if(strcmp(data_array[j].identifier, identifier_use) == 0){
            
            //this is the case if the identifier hasn't been used before
            if (data_array[j].uses == NULL) {
                data_array[j].uses = malloc(sizeof(struct node));
                data_array[j].uses->line_used = lineCount;
                data_array[j].uses->next_use = NULL;
            }
            
            else{
                current = data_array[j].uses;
                
                //this is the case if there has only been one identifier use
                if (current->next_use==NULL) {
                    if (current->line_used == lineCount) {
                        return;
                    }
                    current->next_use = malloc(sizeof(struct node));
                    current->next_use->line_used = lineCount;
                    current->next_use->next_use = NULL;
                }
                //this is the case if there are more than 2 identifier uses
                //it loops to the end of the linked list and adds the new identifer use to the end
                else{
                    while (current->next_use!=NULL) {
                        if (current->next_use->line_used==lineCount) {
                            return;
                        }
                        current = current->next_use;
                    }
                    current->next_use = malloc(sizeof(struct node));
                    current->next_use->line_used = lineCount;
                    current->next_use->next_use = NULL;
                }
            }
        }
     j++;
    }
}

//addToData simply adds the token of the identifier declaration and adds it to data_array
void addToData(char* token, int lineDef){
    strcpy(data_array[i].identifier, token);
    data_array[i].line_defined = lineDef;
    i++; //this increments, keeping a count of the position in the array
}


//printCrossTable prints the data stored in data_array and its linked lists
void printCrossTable(FILE* filepath){
    int count = 0;
    struct node* current;
    fprintf(filepath, "Cross Reference Table:\nIdentifier\tDefinition\tUse\n");
    //loops through every element in the array and prints out its data
    while(count<i){
            fprintf(filepath, "%s\t\t%d\t\t",data_array[count].identifier,data_array[count].line_defined);
        current = data_array[count].uses;
        //loops through the linked list of the element and prints out the identifier uses
        while(current != NULL){
            fprintf(filepath,"%d ",current->line_used);
            current = current->next_use;
        }
        fprintf(filepath, "\n");
        count++;
    }
}


//compareToArray loops through the array and compares the token to the identifer names
int compareToArray(char* token){
    int j = 0;
    while (j<i) {
        if(strcmp(token, data_array[j].identifier))
            //if there is a match it returns 0
            return 0;
        j++;
    }
    //if there is no match, it returns 1
    return 1;
}

//allWhiteSpace tests if the line is all whitespace
int allWhiteSpace(char line[80]){
    int j = 0;
    int ln = strlen(line);

    //it loops through each element in the array
    while (line[j] != '\n' && j<ln) {
        //if the line is not a space, return 0
        if(isspace(line[j])==0){
            return 0;
        }
        j++;
    }
    //if it looped through and it is all whitespace, return 1
    return 1;
}



