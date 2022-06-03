struct Nonogram
// A nonogram board
{
    // The numbers alongside the edges of the nonogram
    std::vector<std::vector<int>> rowNums, colNums;
    // The dimensions of the nonogram
    int rows, cols;
    // The tiles inside the nonogram (rows x cols)
    std::vector<std::vector<bool>> grid;
    
    // Initializes a nonogram from a board string
    // Format: r/o/w/s|c/o/l/s
    // i.e. 3/1/3/1/1|0/1 1/1 1/5/0
    Nonogram(const std::string &boardStr)
    {
        std::vector<int> curr;  // The current row/col
        int i = 0;  // The position in the boardStr

        // Read rowNums
        while (true)
        {
            if (boardStr[i] == '|')
            // Reached end of rowNums
            {
                rowNums.push_back(curr);
                break;
            }
            if (boardStr[i] == '/')
            // Reached end of current rowNum
            {
                rowNums.push_back(curr);
                curr.clear();
                ++i;
            }
            else if (boardStr[i] == ' ')
                ++i;
            else
            // Parse the number and add it to curr
            {
                std::string numStr;
                while (std::isdigit(boardStr[i]))
                    numStr += boardStr[i++];
                curr.push_back(std::stoi(numStr));
            }
        }

        ++i;
        curr.clear();

        // Read colNums
        while (i < boardStr.length())
        {
            if (boardStr[i] == '/')
            // Reached end of current colNum
            {
                colNums.push_back(curr);
                curr.clear();
                ++i;
            }
            else if (boardStr[i] == ' ')
            {
                ++i;
            }
            else
            // Parse the number and add it to curr
            {
                std::string numStr;
                while (i < boardStr.length() && std::isdigit(boardStr[i]))
                    numStr += boardStr[i++];
                curr.push_back(std::stoi(numStr));
            }
        }
        colNums.push_back(curr);
        
        // Store dimensions
        rows = rowNums.size();
        cols = colNums.size();

        // Create appropriate grid
        grid.resize(rows, std::vector<bool>(cols));
    }
    
    // Clears the nonogram grid
    void clearGrid()
    {
        grid.assign(rows, std::vector<bool>(cols));
    }

    // Returns a string representation of the board
    std::string toString() const
    {
        std::string res;

        for (const std::vector<bool> &row : grid)
        {
            for (bool sq : row)
                // □ if empty else ■
                res += sq ? "\u25A0 " : "\u25A1 ";
            res.pop_back();
            res += "\n";
        }
        res.pop_back();

        return res;
    }
};

class NonogramSolver
// A backtracking nonogram solver
{
    bool debug;
    Nonogram *nonogram;
    // The indexes of the current num for each row/col
    std::vector<int> rowNumIdxs, colNumIdxs,
    // The number of tiles in the current block for each row/col
    rowNumPlaced, colNumPlaced;
    
    // Check for legality before placing a tile
    bool isLegal(int row, int col, bool val) const
    {
        if (val && (rowNumPlaced[row] >= nonogram->rowNums[row][rowNumIdxs[row]] ||
            colNumPlaced[col] >= nonogram->colNums[col][colNumIdxs[col]]))
            // Placing the tile would make a block greater than the current nums
            return false;
        
        if (!val && (rowNumPlaced[row] > 0 && rowNumPlaced[row] < nonogram->rowNums[row][rowNumIdxs[row]] ||
            colNumPlaced[col] > 0 && colNumPlaced[col] < nonogram->colNums[col][colNumIdxs[col]]))
            // Not placing the tile would make a block less than the current nums
            return false;
        
        return true;
    }
    
    // Check for legality after placing a tile
    bool isIllegalEnd(int row, int col) const
    {
        // Whether the end of the row/col was reached without the last num being completed
        return col == nonogram->cols - 1 && !(rowNumIdxs[row] == nonogram->rowNums[row].size() - 1 && rowNumPlaced[row] == nonogram->rowNums[row].back()) ||
            row == nonogram->rows - 1 && !(colNumIdxs[col] == nonogram->colNums[col].size() - 1 && colNumPlaced[col] == nonogram->colNums[col].back());
    }
    
    // Step through legal moves until exhausted or a solution is found
    bool backtrack(int row, int col)
    {
        if (++col == nonogram->cols)
        // Reached end of row
        {
            if (++row == nonogram->rows)
                // Reached end of grid, aka solution
                return true;
            col = 0;
        }
        
        for (bool val : { false, true })
        // Try both values
        {
            if (isLegal(row, col, val))
            {
                nonogram->grid[row][col] = val;
                
                if (val)
                {
                    ++rowNumPlaced[row];
                    ++colNumPlaced[col];
                }
                else
                {
                    if (col > 0 && nonogram->grid[row][col - 1] && rowNumIdxs[row] < nonogram->rowNums[row].size() - 1)
                    // Finished a horizontal block
                    {
                        ++rowNumIdxs[row];
                        rowNumPlaced[row] = 0;
                    }
                    if (row > 0 && nonogram->grid[row - 1][col] && colNumIdxs[col] < nonogram->colNums[col].size() - 1)
                    // Finished a vertical block
                    {
                        ++colNumIdxs[col];
                        colNumPlaced[col] = 0;
                    }
                }
                
                if (debug)
                    std::cout << nonogram->toString() << "\n" << row << " " << col << " " << rowNumIdxs[row] << " " << colNumIdxs[col] << " " << rowNumPlaced[row] << " " << colNumPlaced[col] << "\n\n";

                if (!isIllegalEnd(row, col) && backtrack(row, col))
                    // Found a solution
                    return true;

                nonogram->grid[row][col] = false;  // Undo value
                
                if (val)
                {
                    --rowNumPlaced[row];
                    --colNumPlaced[col];
                }
                else
                {
                    if (col > 0 && nonogram->grid[row][col - 1] && rowNumPlaced[row] == 0)
                        // Reverting to previous horizontal block
                        rowNumPlaced[row] = nonogram->rowNums[row][--rowNumIdxs[row]];
                    if (row > 0 && nonogram->grid[row - 1][col] && colNumPlaced[col] == 0)
                        // Reverting to previous vertical block
                        colNumPlaced[col] = nonogram->colNums[col][--colNumIdxs[col]];
                }
            }
        }
        
        return false;  // No solution found
    }
    
    public:
    NonogramSolver(Nonogram &nonogram) : nonogram(&nonogram)
    // Initializes the solver with a nonogram board
    {
        rowNumIdxs.resize(nonogram.rowNums.size());
        colNumIdxs.resize(nonogram.colNums.size());
        rowNumPlaced.resize(nonogram.rowNums.size());
        colNumPlaced.resize(nonogram.colNums.size());
    }

    // Runs backtracking in-place to find a solution
    void solve(bool debug = false)
    {
        this->debug = debug;

        if (!backtrack(0, -1))
            throw std::invalid_argument("Unsolvable Board!");
    }
};

int main()
{
    std::string boardStr;
    std::getline(std::cin, boardStr);
    Nonogram nonogram(boardStr);
    
    auto start = chrono::steady_clock::now();
    NonogramSolver(nonogram).solve(false);
    auto end = chrono::steady_clock::now();
    std::cout << chrono::duration<double, milli>(end - start).count() << " ms\n";
    std::cout << nonogram.toString();
}

/*
Board strings to try:
1/1 1 1/2 2/3 2/3 3/7/1 2/5/2/2|1/1 4/7/3 1 1/1 3/1 2/4/4/3/2

1 1/4/4/1 4/1 2/1 4/1 5/1 5/7/0|0/4/1 1/3/4 4/8/8/4 4/1/0

3/2 2/1 1 1/2 2/3|3/2 2/1 1 1/2 2/3

1/0/1/2 1/2 3/7/2 6/3 1/4/0|2 1/4/2/2/4/5/1 5/2 1/1 3/0

1/1 1/1/1 1 1/3|1/1 1/1 3/1 1/1

0/2/2/3/1 2|2/4/1/2/1

3/1/3/1/1|0/1 1/1 1/5/0

0/3/2/1 1/1 1/8/3 2 1/2 3 2/2 8 1/2 7 1 1/3 6 2 1/3 6 4 1/1 5 4 1/3 3 2/5 3 4 1/1 5 3 3 2/2 3 7 2/2 7 2/3 3/9|2 2/1 2/2 2 2/1 1 3 1/2 1 3 2/2 2 4 1/2 3 2 1 1/2 4 2 1/1 4 4 1/2 4 5 1/1 5 5 1/1 4 1 2 1/1 3 3 2/1 2 6 1/2 1 5 2/2 1 5 2/4 4 2/2 3/1 2 7/1 2

12/2 1 1 1 1 1 3/2 1 1 1 1 1 1 4/1 1 1 1 1 1 1 1 1 4/1 1 1 1 1 1 1 1 1 1 3/1 14 1 1 2/1 3 1 3/12 2 1 3/4 3 1 1/4 2 1 2/8 4 2 3/3 4 2 1 1 2/2 6 2 3 2 3/2 15 3 1/16 3 1 3/18 2 3 1/18 1 1 3/1 16 1 3 1/1 12 1 1 3/18 1 3 1/1 2 10 1 1 1 3/1 2 10 3 1/1 2 3 3 1 3 1/1 2 3 3 2 2/1 2 2 4 2/3 3 2 5 3/1 3 4 4 2/1 4 2 8/1 3 3 1 6/6 1 3|0/1/1 2 5 4/2 1 4 3 4/1 2 1 6 1 1 1/2 1 1 2 4 2 3/1 3 1 1 4 2 2 3/2 1 1 2 5 1 9/1 4 1 1 9 4 1/2 1 1 1 9 2 2/1 4 1 1 12 2/2 1 1 1 17/1 4 1 1 15/2 1 1 12 1/1 4 2 12 2/2 1 3 1 13 2/1 4 20/2 2 3 17/4 1 2 7 1 6/2 3 4 9 4/3 2 4 2 3/2 1 2 4 2 2/3 3 7 4/2 1 3 1 3/3 1 10/4 1 1 1 1 1 2/2 1 1 1 1 1 1/4 1 1 1 1 1 2/4 1 1 1 1 1/12
*/
