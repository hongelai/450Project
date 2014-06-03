/***********************************
void getStrSegment(char* s, char* t,int idx)
function: get the content of the string, the content is seperated by a space
para:
	s: input string
	t: output string
	idx: which segment to get
************************************/
void getStrSegment(char* s, char* t,int idx)
{
  short len=0, Index=0;
  short i=0,j=0;
  char tmp[32];
  
  len = strlen(s)+1;
  strcpy(tmp,s);
  for (i=0;i<len;i++){
	if (s[i] == ' ' || s[i] == '\n' || s[i] == '\t')  tmp[i] = '\0';
  }
  j=0;
  for (i=0;i<len;i++){
	t[j] = tmp[i];
	j++;
	if (tmp[i] == '\0'){
	  Index++;
	  if (Index == idx) return;
	  else j = 0;
	}
  }
}
/***********************************
void stripHeader(char* str)
function: strip all the character before #
para:
	s: input string
	t: output string
	idx: which segment to get
************************************/
void stripHeader(char* str)
{
     int i=0;
      for(i=0; i< strlen(str);i++)
          if(str[i]=='#') break;
      if(i>=strlen(str)) return;
      else{
           char* tok =strtok(str,"#");
           tok = strtok(NULL,"#");
           int j=0;
           for( j =0;j<strlen(tok);j++)
               str[j]=tok[j];
           str[j]='\0';
      } 
}
/**Add end flag to a string**/
void strHandle(char*str)
{
	int i=0;
	for(i=0;isalnum(str[i]);i++);
	str[i]='\0';
}
/**Add end flag to a string**/
void strHandle_phase2(char*str)
{
	int i=0;
	for(i=0;str[i]!='\n';i++);
	str[i]='\0';
}

/*convert integer to string*/
char* itostr(char *str, int i) 
{
    sprintf(str, "%d", i);
    return str;
}
/* let str out put line by line*/
void strPresentHandle(char* str)
{
	int i=0;
	for(i=0;i<strlen(str);i++)
        if(str[i]=='#')
            str[i]='\n';
}
