#include <bits/stdc++.h>

using namespace std;

vector<vector<string> > production;
set<string> terminal;
map<string, int> terminal_map;
map<string, int> non_terminals_map;
map<string,set<string> > first;
map<string,set<string> > follow;
vector<string> input;

vector<string> split(string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

//function to check whether the given lexeme is operator or not
bool isOperator(char ch, int &i) {
	ifstream opt;
	opt.open("myOperator.txt", ios::in);

	if(!opt) {
		cout<<"Error while opening the operator file";	exit(0);
	}

	i=0;
	while(opt.good()) {
		string hel;
		getline(opt, hel);
		if(ch == hel[0])	return true;
		i++;
	}

	//not found. Closing the file...
    opt.close();
    return false;
}

//function to check whether the given lexeme is white space or not
bool isSpace(char ch) { return (ch==' '||ch=='\t');}

//function to check whether the given lexeme is integer or not
bool isInt(char ch) {return (ch>='0' && ch<='9');}

//function for extracting tokens from input file
void extract_tokens()
{
    //opening the input file for reading
    ifstream input_file;
    input_file.open("input.txt", ios::in);
    if(!input_file) {
        cout<<"Error occurred while opening the file";  exit(0);
    }

    //main loop for reading the file
    cout<<"The input program is:"<<endl;
    while(input_file.good()) {
        string go, nop;
        getline(input_file, go);        cout<<go<<endl;
        int i, j;
        for(i=0;i<go.size();i++) {
        	//for white space
            if(isSpace(go[i]))      continue;

            //for multi line comments
            else if(go[i]=='/' && go[i+1]=='*') {
                i+=2;
                while(i<go.size()-1 && go[i]!='*' && go[i+1]!='/')  i++;
                i++;
            }

            //for single line comment
            else if(go[i]=='/' && go[i+1]=='/')		break;

            //for operators
            else if(isOperator(go[i], j)) {
                string chk="";      chk+=go[i];
                if((go[i] == '>' || go[i] == '<') && isOperator(go[i+1], j)) {
                    chk+=go[i+1];   i++;
                }
                input.push_back(chk);
            }

			else {
				//constructing the given word in a string
                string cons="";     bool flag=false;

                while(i<go.size()) {
                    if(isSpace(go[i]) || isOperator(go[i], j)) {i--; break;}
                    cons += go[i];  i++;
                }

                //checking if the constructed string is an integer constant
                if(isInt(cons[0])) {
                	int p=1;
                	while(p<cons.size() && isInt(cons[p]))	p++;
                	if(p<cons.size())	{
                		cout<<"\n\n\t\tTHERE IS ERROR IN YOUR CODE\n\n";    exit(0);
					}
					input.push_back("integer");     flag=true;
				}

				if(!flag)  {
                    if(terminal.find(cons) != terminal.end())
                        input.push_back(cons);
                    else    input.push_back("id");
				}
			}
        }
    }
    input_file.close();	//closing the input file
}

set<string> find_first(string ch, int no_of_production)
{
    set<string> arr, dummy;
    if(terminal.find(ch) != terminal.end()) {
        arr.insert(ch);     return arr;
    }
    for(int i=0;i<no_of_production;i++) {
        //Find production with X as LHS
        if(production[i][0]==ch) {
            //If X : epsilon is a production, then add epsilon to FIRST(X).
            if(production[i][2]=="^")   arr.insert("^");
            //If X is a non-terminal, and X : Y1 Y2 … Yk
            //is a production, then add a to FIRST(X)
            //if for some i, a is in FIRST(Yi),
            //and epsilon is in all of FIRST(Y1), …, FIRST(Yi-1).
            else {
                for(int j=2; j<production[i].size();j++) {
                    bool foundEpsilon=false;
                    dummy=find_first(production[i][j], no_of_production);
                    for(set<string>::iterator it=dummy.begin();it!=dummy.end();it++) {
                        arr.insert(*it);
                        if((*it) == "^")    foundEpsilon=true;
                    }
                    if(!foundEpsilon)   break;
                }
            }
        }
    }
    return arr;
}

set<string> find_follow(string ch, int no_of_production)
{
      set<string> arr, dummy;

      //If it is the start symbol
      if(production[0][0] == ch)    arr.insert("$");

      for(int i = 0; i < no_of_production; i++)
      {
            for(int j = 2; j < production[i].size(); j++)
            {
                  if(production[i][j] == ch)
                  {
                        if(j+1 < production[i].size()) {
                              dummy = first[production[i][j + 1]];
                              for(set<string>::iterator it=dummy.begin();it!=dummy.end();it++) {
                                    if(*it != "^")   arr.insert(*it);
                              }
                              for(set<string>::iterator it=dummy.begin();it!=dummy.end();it++) {
                                    if(*it == "^") {
                                        if(ch == production[i][0])  break;
                                        set<string> temp = find_follow(production[i][0], no_of_production);
                                        for(set<string>::iterator g=temp.begin();g!=temp.end();g++)  arr.insert(*g);
                                    }
                              }
                        }
                        if(j+1 == production[i].size() && ch != production[i][0]) {
                            dummy = find_follow(production[i][0], no_of_production);
                            for(set<string>::iterator it=dummy.begin();it!=dummy.end();it++) {
                                if(*it != "^")  arr.insert(*it);
                            }
                        }
                  }
            }
      }
      return arr;
}

vector<vector<string> > create_ll1_parsing_table(int no_of_production) {

    //Storing the non-terminal symbols
    int ind=1;
    for(int i=0;i<production.size();i++) {
        if(non_terminals_map.find(production[i][0]) == non_terminals_map.end()) {
            non_terminals_map[production[i][0]] = ind;      ind++;
        }
    }

    //LL(1) Parsing Table
    vector<vector<string> > table(non_terminals_map.size()+1, vector<string>(terminal.size()+1, "_"));

    //Creating first row of terminals
    ind=1;
    for(set<string>::iterator it=terminal.begin(); it!=terminal.end();it++) {
        terminal_map[*it]=ind;
        table[0][ind] = *it;        ind++;
        set<string> visit;
        visit.insert(*it);
        first[*it]=visit;
    }

    //ind=1;
    for(map<string, int>::iterator it=non_terminals_map.begin(); it!=non_terminals_map.end();it++) {
        table[it->second][0] = it->first;
        for(int i=0;i<no_of_production;i++) {
            if(production[i][0] == it->first) {
                string prod_rule="";
                for(int j=2;j<production[i].size();j++)
                    prod_rule += production[i][j] + " ";

                set<string> first_set = first[production[i][2]];

                //cout<<"All first of "<<production[i][0]<<": ";
                //for(set<string>::iterator iter=first_set.begin(); iter!=first_set.end();iter++) cout<<*iter<<" ";
                //cout<<endl;

                for(set<string>::iterator iter=first_set.begin(); iter!=first_set.end();iter++) {
                    //cout<<"Current nt: "<<production[i][0]<<" "<<*iter<<endl;
                    //if(*iter == "^")    cout<<"^ found"<<endl;
                    if(terminal_map.find(*iter) != terminal_map.end()) {
                        table[it->second][terminal_map[*iter]] = prod_rule;
                    }
                    else if(*iter == "^") {
                        //cout<<"nt: "<<production[i][0]<<"Production rule: "<<prod_rule<<endl;
                        set<string> follow_set = follow[production[i][0]];
                        for(set<string>::iterator myiter=follow_set.begin(); myiter!=follow_set.end();myiter++) {
                            //cout<<"follow set: "<<*myiter<<endl;
                            if(*myiter != "^")
                                table[it->second][terminal_map[*myiter]] = prod_rule;
                        }
                    }  //checking epsilon
                }//first set
            }  //finding non-terminal
        }  //checking all the production
    }

    ofstream parsing_table;
    parsing_table.open("parsing_table.txt", ios::out);
    for(int i=0;i<table.size();i++) {
        for(int j=0;j<table[i].size();j++)  parsing_table<<table[i][j]<<"\t|\t";
        parsing_table<<endl;
    }
    parsing_table.close();

    return table;
}

void parsing(vector<vector<string> > table) {
    vector<string> s;
    s.push_back("$");
    s.push_back(production[0][0]);

    ofstream parse;
    parse.open("parsing_action.txt", ios::out);
    parse<<"STACK ACTION\t\t\t\t\t\t\t\t\t\t\tINPUT TO BE PARSED"<<endl;

    int input_index=0;

    while(!s.empty() && input_index < input.size()) {
        int stack_size=s.size();
        for(int i=0;i<stack_size;i++)     parse<<s[i]<<" ";   parse<<"\t\t"<<input[input_index]<<endl;
        if(s[stack_size-1] == "$" && input[input_index] == "$") {
            parse<<"Input Accepted...";  cout<<"Input Accepted..."<<endl;   break;
        }

        //for(int i=input_index;i<input.size();i++)   parse<<input[i]<<" ";   parse<<endl;
        if(s[stack_size-1] == "^") {
            s.pop_back();    continue;
        }
        if(terminal.find(s[stack_size-1]) != terminal.end()) {
            if(s[stack_size-1] == input[input_index]) {
                //cout<<"Symbol to be popped: "<<s[stack_size-1]<<endl;
                s.pop_back();    input_index++;
                //cout<<"Current stack: "<<s[stack_size-1]<<" Input: "<<input[input_index]<<endl;
            }
            else {
                cout<<"Stack top: "<<s[stack_size-1]<<" Input: "<<input[input_index]<<endl;
                cout<<"Error in parsing";   exit(0);
            }
        }
        else {
            int row_ind = non_terminals_map[s[stack_size-1]];
            int col_ind = terminal_map[input[input_index]];
            string to_be_replaced = table[row_ind][col_ind];
            s.pop_back();
            vector<string> arr=split(to_be_replaced,' ');
            for(int i=arr.size()-1;i>=0;i--)
                s.push_back(arr[i]);
        }
        //break;
    }
    parse.close();
}

int main()
{
    ifstream fin;
    fin.open("cfg_left_recursion_removed.txt",ios::in);
    if(!fin) {
        cout<<"Error while opening the file"<<endl; exit(0);
    }
    while(fin.good()) {
        string go;
        getline(fin,go);
        vector<string> arr=split(go,' ');
        production.push_back(arr);
    }

    ifstream f;
    f.open("terminal.txt",ios::in);
    if(!f) {
        cout<<"Error while opening the file"<<endl; exit(0);
    }
    while(f.good()) {
        string go;      getline(f,go);  terminal.insert(go);
    }


    int no_of_production = production.size();
    for(int i=0;i<no_of_production;i++) {
        if(i>1 && production[i][0] == production[i-1][0])   continue;
        set<string> ans = find_first(production[i][0], no_of_production);
        first[production[i][0]] = ans;
    }

    //Constructing the FIRST set
    ofstream out;
    out.open("first.txt", ios::out);
    out<<"FIRST of Terminal symbols"<<endl;
    for(int y=0;y<50;y++)  out<<"-";   out<<endl;
    for(set<string>::iterator it=terminal.begin();it!=terminal.end();it++) {
        out<<"FIRST("<<*it<<"): ["<<*it<<"]"<<endl;
    }

    out<<endl<<"FIRST of Non-terminal symbols"<<endl;
    for(int y=0;y<50;y++)  out<<"-";   out<<endl;
    for(map<string,set<string> >::iterator it=first.begin();it!=first.end();it++) {
        set<string> hel=it->second;     out<<"FIRST("<<it->first<<"): [";
        /*if(it->first == "exp'") {
            cout<<"first of "<<it->first<<": ";
            for(set<string>::iterator it1=hel.begin();it1!=hel.end();it1++)
            cout<<*it1<<" ";
        }*/
        for(set<string>::iterator it1=hel.begin();it1!=hel.end();it1++) {
            out<<*it1<<" ";
        }
        out<<"]"<<endl;
    }
    out.close();
    cout<<"FIRST of all Terminal and Non-Terminal symbols created in 'first.txt'"<<endl;

    set<string> eps;    eps.insert("^");
    first["^"]=eps;

    for(set<string>::iterator it=terminal.begin();it!=terminal.end();it++) {
        set<string> vis;
        vis.insert(*it);
        first[*it]=vis;
    }


    //Constructing the FOLLOW set
    ofstream fol;
    fol.open("follow.txt", ios::out);


    for(int i=0;i<no_of_production;i++) {
        if(i>1 && production[i][0] == production[i-1][0])   continue;
        set<string> ans = find_follow(production[i][0], no_of_production);
        follow[production[i][0]] = ans;
    }

    for(map<string,set<string> >::iterator it=follow.begin();it!=follow.end();it++) {
        set<string> hel=it->second;     fol<<"FOLLOW("<<it->first<<"): [";
        for(set<string>::iterator it1=hel.begin();it1!=hel.end();it1++) {
            fol<<*it1<<" ";
        }
        fol<<"]"<<endl;
    }
    fol.close();
    cout<<"FOLLOW of all Non-Terminal symbols created in 'follow.txt'"<<endl;

    vector<vector<string> > table = create_ll1_parsing_table(no_of_production);
    cout<<"LL(1) parsing table created in 'parsing_table.txt'"<<endl;

    extract_tokens();
    input.push_back("$");
    //for(int i=0;i<input.size();i++)     cout<<input[i]<<endl;
    parsing(table);
    cout<<"All the parsing actions and stack contents are shown in 'parsing_action.txt'"<<endl;

    return 0;
}
