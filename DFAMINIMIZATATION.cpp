#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm> 

using namespace std;

int numStates, numLetters, numFinal, *FinalStates;
std::vector<int> **Partitions;
int **transitionFunction;

// Function to find the partition index for a given state
vector<int> findPartition(int state, int partitionIndex, int currentPartition)  
{  
    int i;
    vector<int> partitionIndices;
    int size, j;
    for(int k = 0; k < numLetters; k++)
    {
        int nextState = transitionFunction[state][k];
        for(i = 0; i < currentPartition + 1; i++)
        { 
            size = Partitions[partitionIndex][i].size();
            for(j = 0; j < size; j++)
            {
                if(nextState == Partitions[partitionIndex][i][j])
                {
                    partitionIndices.push_back(i);
                }
            }
        }
    }
    return partitionIndices;
}

// Function to check if the current partitioning is stable
int checkPartitions(int partitionIndex, int currentPartition)
{ 
    int isStable = 1, j, k1, k2, i;
    for(i = 0; i < currentPartition + 1; i++)
    {   
        int size = Partitions[partitionIndex][i].size();
        vector<int> partitionsTemp[20];
        
        for(j = 0; j < size; j++)
        { 
            int state = Partitions[partitionIndex][i][j];
            partitionsTemp[j] = findPartition(state, partitionIndex, currentPartition);
        }
        
        for(k1 = 0; k1 < size; k1++)
            for(k2 = 0; k2 < size; k2++)
                if(partitionsTemp[k1] != partitionsTemp[k2])
                    isStable = 0;
    }
    return isStable;
}

// Function to generate DOT file for the minimized DFA
void generateDotFile(int currentPartitionCount, int contor1, int numLetters, char Alphabet[], vector<int> **Partitions, int **transitionFunction, int initialState, int numFinal, int FinalStates[]) {
    ofstream dotFile("minimized_dfa.dot");
    if (!dotFile.is_open()) {
        cout << "Error: Unable to create DOT file.\n";
        return;
    }

    dotFile << "digraph DFA {\n";
    dotFile << "    rankdir=LR;\n";
    dotFile << "    size=\"8,5\"\n";
    
    // Define node shapes for final states
    dotFile << "    node [shape = doublecircle];";
    // Collect final states in the minimized DFA
    vector<int> minimizedFinalStates;
    for(int k = 0; k < numFinal; k++) {
        int finalState = FinalStates[k];
        // Find which minimized state contains this final state
        for(int i = 0; i < currentPartitionCount + 1; i++) {
            for(int j = 0; j < Partitions[contor1][i].size(); j++) {
                if(Partitions[contor1][i][j] == finalState) {
                    minimizedFinalStates.push_back(i);
                    break;
                }
            }
        }
    }
    // Remove duplicates
    vector<int> uniqueFinalStates;
    for(auto state : minimizedFinalStates) {
        if(find(uniqueFinalStates.begin(), uniqueFinalStates.end(), state) == uniqueFinalStates.end()) {
            uniqueFinalStates.push_back(state);
        }
    }
    for(auto state : uniqueFinalStates) {
        dotFile << " q" << state;
    }
    dotFile << ";\n";

    // Define node shapes for non-final states
    dotFile << "    node [shape = circle];\n";

    // Define transitions
    for(int i = 0; i < currentPartitionCount + 1; i++) {
        if(Partitions[contor1][i].empty()) continue;
        int representative = Partitions[contor1][i][0]; // Choose the first state as representative
        for(int j = 0; j < numLetters; j++) {
            int nextState = transitionFunction[representative][j];
            if(nextState == -1) continue; // No transition
            // Find the minimized state index for nextState
            int minimizedNextState = -1;
            for(int m = 0; m < currentPartitionCount + 1; m++) {
                if(find(Partitions[contor1][m].begin(), Partitions[contor1][m].end(), nextState) != Partitions[contor1][m].end()) {
                    minimizedNextState = m;
                    break;
                }
            }
            if(minimizedNextState != -1) {
                dotFile << "    q" << i << " -> q" << minimizedNextState << " [ label = \"" << Alphabet[j] << "\" ];\n";
            }
        }
    }

    // Indicate the initial state with an incoming arrow
    dotFile << "    // Initial state indicator\n";
    dotFile << "    init [shape = point];\n";
    // Find the minimized state containing the initial state
    int minimizedInitialState = -1;
    for(int i = 0; i < currentPartitionCount + 1; i++) {
        if(find(Partitions[contor1][i].begin(), Partitions[contor1][i].end(), initialState) != Partitions[contor1][i].end()) {
            minimizedInitialState = i;
            break;
        }
    }
    if(minimizedInitialState != -1) {
        dotFile << "    init -> q" << minimizedInitialState << ";\n";
    }

    dotFile << "}\n";
    dotFile.close();
    cout << "DOT file 'minimized_dfa.dot' has been generated.\n";
    cout << "Use Graphviz to visualize it, e.g., run:\n";
    cout << "    dot -Tpng minimized_dfa.dot -o minimized_dfa.png\n";
    cout << "    open minimized_dfa.png\n"; // 'open' works on macOS; adjust for other OS if needed
}

int main()
{   
    // Total states
    cout << "How many states does the automaton have?\n"; 
    cin >> numStates;
    int Q[numStates], i, initialState;
    for(i = 0; i < numStates; i++) 
        Q[i] = i;

    // Alphabet
    cout << "How many letters are in the alphabet?\n"; 
    cin >> numLetters;
    char Alphabet[numLetters], c;
    for(i = 0; i < numLetters; i++)
    {
        cout << "Enter symbol " << i+1 << ": ";
        cin >> c;
        Alphabet[i] = c;
    }

    // Initial state
    cout << "What is the initial state? "; 
    cin >> initialState;

    // Final states
    cout << "How many final states does the automaton have?\n"; 
    cin >> numFinal;
    FinalStates = new int[numFinal];
    cout << "What are the final states?\n";
    for(i = 0; i < numFinal; i++)
    {
        cin >> FinalStates[i];
    }

    int z, y;
    // Transition function
    transitionFunction = new int*[numStates];
    for(i = 0; i < numStates; i++)
        transitionFunction[i] = new int[numLetters];
    
    cout << "Transition function:\n";
    for(i = 0; i < numStates; i++) {
        for(int j = 0; j < numLetters; j++) {
            cout << "State " << Q[i] << " --" << Alphabet[j] << "--> \n";
            cout << "Enter the number of states it transitions to:\n";
            cin >> z;
            if(z == 1)
            {
                cout << "Which state does it transition to?\n";
                cin >> y;
                transitionFunction[i][j] = y;
            }
            else if(z == 0) 
                transitionFunction[i][j] = -1;
        }
    }

    y = 0;
    int jIndex, isFinal = 1;
    Partitions = new vector<int>*[20];
    for(i = 0; i < numStates; i++)
        Partitions[i] = new vector<int>[20];
    
    for(i = 0; i < numStates; i++)
    {
        isFinal = 1;
        for(jIndex = 0; jIndex < numFinal; jIndex++)
        {
            if(Q[i] == FinalStates[jIndex])
                isFinal = 0;
        }
        if(isFinal == 1) 
        {
            Partitions[0][0].push_back(Q[i]); 
            y++;
        }
    }
    
    for(i = 0; i < numFinal; i++)
        Partitions[0][1].push_back(FinalStates[i]);
    
    cout << "Partitions[0]:\n";
    for(i = 0; i < 2; i++)
    { 
        int size = Partitions[0][i].size();
        for(jIndex = 0; jIndex < size; jIndex++)
            cout << " q" << Partitions[0][i][jIndex];
        cout << "\n";
    }

    int currentPartitionIndex = 0, currentPartitionCount = 1, newPartitionIndex, newPartitionCount, l1Index;
    int stability;
    stability = checkPartitions(currentPartitionIndex, currentPartitionCount);
    
    if(stability != 0) 
    {
        cout << "The automaton is already minimal!\n"; 
        // Still generate DOT file for the current DFA
        generateDotFile(currentPartitionCount, currentPartitionIndex, numLetters, Alphabet, Partitions, transitionFunction, initialState, numFinal, FinalStates);
        return 0;
    }
    
    while(stability == 0)
    {
        newPartitionIndex = currentPartitionIndex + 1;
        newPartitionCount = 0;
        for(int k1 = 0; k1 < currentPartitionCount + 1; k1++)
        {
            int m = newPartitionCount;
            Partitions[newPartitionIndex][newPartitionCount].push_back(Partitions[currentPartitionIndex][k1][0]);
            int size = Partitions[currentPartitionIndex][k1].size();
            for(i = 1; i < size; i++)
            {  
                vector<int> partitionsList;
                partitionsList = findPartition(Partitions[currentPartitionIndex][k1][i], currentPartitionIndex, currentPartitionCount);
                isFinal = 0;
                for(int i1 = m; i1 < newPartitionCount + 1; i1++)
                {   
                    vector<int> referenceList;
                    referenceList = findPartition(Partitions[newPartitionIndex][i1][0], currentPartitionIndex, currentPartitionCount);
                    if(partitionsList == referenceList)
                    {
                        isFinal = 1;
                        Partitions[newPartitionIndex][i1].push_back(Partitions[currentPartitionIndex][k1][i]);
                        break;
                    }
                }
                if(isFinal == 0)
                {
                    newPartitionCount++;
                    Partitions[newPartitionIndex][newPartitionCount].push_back(Partitions[currentPartitionIndex][k1][i]);
                }
            }
            if(k1 + 1 < currentPartitionCount + 1) 
                newPartitionCount++;
        }
        currentPartitionIndex = newPartitionIndex;
        currentPartitionCount = newPartitionCount;
        stability = checkPartitions(currentPartitionIndex, currentPartitionCount);
        cout << "Partitions[" << currentPartitionIndex << "]:\n";
        for(i = 0; i < currentPartitionCount + 1; i++)
        {
            int size = Partitions[currentPartitionIndex][i].size();
            for(jIndex = 0; jIndex < size; jIndex++)
                cout << " q" << Partitions[currentPartitionIndex][i][jIndex] << " ";
            cout << "\n";
        }
    }

    int k;
    // Display minimized automaton
    cout << "THE NUMBER OF STATES IN THE MINIMIZED AUTOMATON IS " << currentPartitionCount + 1 << "\n";
    for(i = 0; i < currentPartitionCount + 1; i++)
        for(jIndex = 0; jIndex < Partitions[currentPartitionIndex][i].size(); jIndex++)
            if(Partitions[currentPartitionIndex][i][jIndex] == initialState)
                cout << "THE INITIAL STATE IS q" << i << "\n";
    cout << "THE FINAL STATES ARE: ";
    for(k = 0; k < numFinal; k++)
        for(i = 0; i < currentPartitionCount + 1; i++)
            for(jIndex = 0; jIndex < Partitions[currentPartitionIndex][i].size(); jIndex++)
                if(FinalStates[k] == Partitions[currentPartitionIndex][i][jIndex])
                    cout << "q" << i << " ";
    cout << "\n";

    for(i = 0; i < currentPartitionCount + 1; i++)
    {
        cout << "q" << i << " transitions through:\n";
        { 
            vector<int> transitionList;
            transitionList = findPartition(Partitions[currentPartitionIndex][i][0], currentPartitionIndex, currentPartitionCount);
            for(int t = 0; t < transitionList.size(); t++)
                cout << char(97 + t) << " to q" << transitionList[t] << "\n";
        }
    }

    // *Generate Visualization*
    generateDotFile(currentPartitionCount, currentPartitionIndex, numLetters, Alphabet, Partitions, transitionFunction, initialState, numFinal, FinalStates);

    // Cleanup dynamically allocated memory
    for(i = 0; i < numStates; i++)
        delete[] transitionFunction[i];
    delete[] transitionFunction;
    delete[] FinalStates;
    for(i = 0; i < 20; i++) {
        delete[] Partitions[i];
    }
    delete[] Partitions;

return 0;
}