#include <stdio.h>
#include <stdlib.h>

#include "SimpleVector.h"


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
class CellCandidateIterator_c;
class Map_c;

class CellInfo_c
{
    friend class CellCandidateIterator_c;

    public:
        CellInfo_c() {}
        ~CellInfo_c() {}

        static void setDimension(int nDimension) { MAP_DIMENSION = nDimension; }

        bool init()
        {
            nData_m     = 0;
            nOccupied_m = 0;
            nFlag_m     = 0;

            return true;
        }

        CellInfo_c& operator=(CellInfo_c const& orig)
        {
            nData_m     = orig.nData_m;
            nOccupied_m = orig.nOccupied_m;
            nFlag_m     = orig.nFlag_m;

            return *this;
        }

        bool setValue(int nValue)
        {
            if (isOccupied())
            {
                return false;
            }

            nData_m = nValue;
            nOccupied_m = 1;

            return true;
        }

        int getValue()          { return nData_m; }
        bool isOccupied()       { return 1 == nOccupied_m; }

        int getFirstCandidate()
        {
            if (isOccupied())
            {
                return 0;
            }

            // check if the cell still has empty slot
            int nCheckMask = 0x01;
            for (int i=0; i<MAP_DIMENSION; i++)
            {
                if (0 == (nCheckMask & nFlag_m))
                {
                    return (i + 1);
                }

                nCheckMask <<= 1;
            }

            return 0;
        }

        int getEmptySlotCount()
        {
            if (isOccupied())
            {
                return 0;
            }

            int nCount = 0;

            // check if the cell still has empty slot
            int nCheckMask = 0x01;
            for (int i=0; i<MAP_DIMENSION; i++)
            {
                if (0 == (nCheckMask & nFlag_m))
                {
                    nCount++;
                }

                nCheckMask <<= 1;
            }

            return nCount;
        }

        bool hasEmptySlot()
        {
            return (getEmptySlotCount() > 0);
        }

        bool setUnavailableFlag(int nValue)
        {
            if ((nValue <= 0) || (nValue > MAP_DIMENSION))
            {
                return false;
            }

            if (isOccupied())
            {
                return true;
            }

            // update slot flag
            int nMask = (0x01 << (nValue - 1));

            nFlag_m |= nMask;

            return hasEmptySlot();
        }

        void print()
        {
            printf("%x %x     ", nOccupied_m, nData_m);

            if (!isOccupied())
            {
                int nCheckMask = 0x01;
                for (int i=0; i<MAP_DIMENSION; i++)
                {
                    if (0 != (nCheckMask & nFlag_m))
                    {
                        printf("%d ", i + 1);
                    }

                    nCheckMask <<= 1;
                }
            }
        }
    protected:

    private:
        static int MAP_DIMENSION;

    private:
        int nData_m:8;
        int nOccupied_m:2;
        int nFlag_m:22;
};

// init static variable
int CellInfo_c::MAP_DIMENSION = 0;

// traverse all available candidate of cell
class CellCandidateIterator_c
{
    public:
        CellCandidateIterator_c(CellInfo_c* pCell, int nX, int nY) :
            pCell_m(pCell),
            nCurIndex_m(0),
            nX_m(nX),
            nY_m(nY)
        {
        }

        ~CellCandidateIterator_c() {}

        int getX()      { return nX_m; }
        int getY()      { return nY_m; }

        int next()
        {
            int nDimension  = 9;

            if ((NULL == pCell_m) || (nCurIndex_m >= nDimension))
            {
                return 0;
            }

            int nMask       = 0x01;
            for (; nCurIndex_m<nDimension;)
            {
                nCurIndex_m++;
                nMask = (0x01 << (nCurIndex_m - 1));
                if (0 == (nMask & pCell_m->nFlag_m))
                {
                    return nCurIndex_m;
                }
            }

            return 0;
        }
    protected:
    private:
        CellInfo_c* pCell_m;

        int nCurIndex_m:8;
        int nX_m:8;
        int nY_m:8;
};

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////


class Map_c
{
    private:
        struct TCell
        {
            int nX_m:8;
            int nY_m:8;
        };

    public:
        Map_c() :
            pCells_m(NULL),
            nKnownCount_m(0),
            lstSolvedCells_m(NULL),
            nProcessingPtr_m(0),
            nSolvedCount_m(0)
        {
        }

        ~Map_c()
        {
            if (NULL != pCells_m)
            {
                delete[] pCells_m;
                pCells_m = NULL;
            }

            if (NULL != lstSolvedCells_m)
            {
                for (int i=lstSolvedCells_m->size()-1; i>=0; i++)
                {
                    delete (lstSolvedCells_m->takeLast());
                }

                delete lstSolvedCells_m;
            }
        }

        static void setDimension(int nX, int nY)
        {
            BLOCK_DIMENSION_X   = nX;
            BLOCK_DIMENSION_Y   = nY;

            MAP_DIMENSION       = BLOCK_DIMENSION_X * BLOCK_DIMENSION_Y;
            CELL_COUNT          = MAP_DIMENSION * MAP_DIMENSION;

            CellInfo_c::setDimension(MAP_DIMENSION);
        }

        Map_c& operator=(Map_c const& orig)
        {
            if (NULL == pCells_m)
            {
                pCells_m = new CellInfo_c[CELL_COUNT];
            }

            for (int i=0; i<MAP_DIMENSION; i++)
            {
                for (int j=0; j<MAP_DIMENSION; j++)
                {
                    *(getCell(i, j)) = orig.getCell(i, j);
                }
            }

            nKnownCount_m = orig.nKnownCount_m;

            return *this;
        }

        bool isSolved()
        {
            return (CELL_COUNT == nKnownCount_m);
        }

        CellCandidateIterator_c* getCandidateCell()
        {
            if (NULL == pCells_m)
            {
                return NULL;
            }

            int nMinPossibleCount = 999;    // initial
            int nX = -1;
            int nY = -1;

            for (int i=0; (i<MAP_DIMENSION) && (nMinPossibleCount>2) ; i++)
            {
                for (int j=0; (j<MAP_DIMENSION) && (nMinPossibleCount>2); j++)
                {
                    int nPossible = getCell(i, j)->getEmptySlotCount();
                    if ((nPossible > 0) && (nPossible < nMinPossibleCount))
                    {
                        nX = i;
                        nY = j;

                        nMinPossibleCount = nPossible;
                    }
                }
            }

            return ((nX >= 0) && (nY >=0)) ? new CellCandidateIterator_c(getCell(nX, nY), nX, nY) : NULL;
        }

        bool setCell(int x, int y, int nValue)
        {
            if (NULL == pCells_m)
            {
                pCells_m = new CellInfo_c[CELL_COUNT];
            }

            lstSolvedCells_m    = new SimpleVector<TCell*>(5);

            int nCurX           = x;
            int nCurY           = y;
            int nSelectedValue  = nValue;
            
            nProcessingPtr_m    = 0;
            nSolvedCount_m      = 0;

            do
            { 
                // 1. update given cell's value
                updateCell(nCurX, nCurY, nSelectedValue);

                // 2. update related cells flag
                if (false == updateRelatedCellFlag(nCurX, nCurY, nSelectedValue))
                {
                    return false;
                }

                // 3) update known cells
                //    During updating related cells, some of them may change to known state, that means
                //    there is one, and only one possible candidate. We'll set these cells in this round
                if ((nSolvedCount_m > 0) && (nProcessingPtr_m < nSolvedCount_m))
                {
                    nCurX = solvedCells_m[nProcessingPtr_m].nX_m;
                    nCurY = solvedCells_m[nProcessingPtr_m].nY_m;

                    nSelectedValue = getCell(nCurX, nCurY)->getFirstCandidate();
                    if (0 == nSelectedValue)
                    {
                        return false;
                    }

                    //printf("Solved one: [%d][%d] = %d\n", nCurX, nCurY, nSelectedValue);

                    nProcessingPtr_m++;
                }
                else
                {
                    nProcessingPtr_m    = 0;
                    nSolvedCount_m      = 0;
                }
            } while ((nSolvedCount_m > 0) && (nProcessingPtr_m <= nSolvedCount_m));

            return true;
        }

        void printMap()
        {
            if (NULL == pCells_m)
            {
                printf("Map is not initialized yet!\n");
            }

            printf("\n");

            for (int j=0; j<MAP_DIMENSION; j++)
            {
                for (int i=0; i<MAP_DIMENSION; i++)
                {
                    printf("%d ", getCell(i, j)->getValue());
                }

                printf("\n");
            }
        }

        void print()
        {
            if (NULL == pCells_m)
            {
                printf("Map is not initialized yet!\n");
            }

            printf("Known count: %d\n", nKnownCount_m);
            for (int i=0; i<MAP_DIMENSION; i++)
            {
                for (int j=0; j<MAP_DIMENSION; j++)
                {
                    printf("[%d][%d]: ", i, j);
                    getCell(i, j)->print();

                    printf("\n");
                }
            }
        }
    protected:
        const CellInfo_c& getCell(int x, int y) const
        {
            return pCells_m[x + y * MAP_DIMENSION];
        }

        CellInfo_c* getCell(int x, int y)
        {
            return &pCells_m[x + y * MAP_DIMENSION];
        }

        void updateCell(int x, int y, int nValue)
        {
            if (getCell(x, y)->setValue(nValue))
            {
                nKnownCount_m++;
            }
        }

        bool updateRelatedCellFlag(int x, int y, int nValue)
        {
            for (int i=0; i<MAP_DIMENSION; i++)
            {
                // update cells on related row
                if ((i != y) && (!setCellUnavailableFlag(x, i, nValue)))
                {
                    return false;
                }

                // update cells on related column
                if ((i != x) && (!setCellUnavailableFlag(i, y, nValue)))
                {
                    return false;
                }
            }

            // update block
            return updateRelatedCellFlagInBlock(x, y, nValue);
        }

        bool updateRelatedCellFlagInBlock(int x, int y, int nValue)
        {
            int nBlockRow = (x / BLOCK_DIMENSION_X) * BLOCK_DIMENSION_X;
            int nBlockCol = (y / BLOCK_DIMENSION_Y) * BLOCK_DIMENSION_Y;

            for (int iRow=0; iRow<BLOCK_DIMENSION_X; iRow++)
            {
                for (int iCol=0; iCol<BLOCK_DIMENSION_Y; iCol++)
                {
                    int nXPos = nBlockRow + iRow;
                    int nYPos = nBlockCol + iCol;
                    if ((nXPos != x) || (nYPos != y))
                    {
                        if (!setCellUnavailableFlag(nXPos, nYPos, nValue))
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        bool setCellUnavailableFlag(int nX, int nY, int nValue)
        {
            CellInfo_c* pCell = getCell(nX, nY);

            if (!pCell->isOccupied())
            {
                if (!pCell->setUnavailableFlag(nValue))
                {
                    return false;
                }

                // check if the cell has been solved
                if (1 == pCell->getEmptySlotCount())
                {
                    appendSolvedCell(nX, nY);
                }
            }

            return true;
        }

        void appendSolvedCell(int x, int y)
        {
            for (int i=0; i<nSolvedCount_m; i++)
            {
                if ((solvedCells_m[i].nX_m == x) && (solvedCells_m[i].nY_m == y))
                {
                    // the cell has been put into queue, return directly
                    return;
                }
            }

            solvedCells_m[nSolvedCount_m].nX_m = x;
            solvedCells_m[nSolvedCount_m].nY_m = y;

            nSolvedCount_m++;
        }

    private:
        static int BLOCK_DIMENSION_X;
        static int BLOCK_DIMENSION_Y;

        // map dimension = BLOCK_DIMENSION_X * BLOCK_DIMENSION_Y
        static int MAP_DIMENSION;
        // cell count    = MAP_DIMENSION * MAP_DIMENSION
        static int CELL_COUNT;

    private:
        CellInfo_c* pCells_m;
        int nKnownCount_m;

        // FIFO queue, will not exceed 81;
        SimpleVector<TCell*> * lstSolvedCells_m;

        TCell solvedCells_m[81];

        int nProcessingPtr_m;
        int nSolvedCount_m;
};

// init static variable
int Map_c::BLOCK_DIMENSION_X    = 0;
int Map_c::BLOCK_DIMENSION_Y    = 0;
int Map_c::MAP_DIMENSION        = 0;
int Map_c::CELL_COUNT           = 0;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
struct SudokuCellStackItem_t
{
    SudokuCellStackItem_t() :
        pCellCandidateIter_m(NULL)
    {
    }
    Map_c map_m;

    //MapAvailableCellIterator_c* pMapAvailIter_m;

    CellCandidateIterator_c* pCellCandidateIter_m;
};


bool initializeSudokuMap(Map_c& sudokuMap)
{
    /*
    // test map 1
    {
        Map_c::setDimension(3, 3);

        sudokuMap.setCell(0, 0, 8);
        sudokuMap.setCell(3, 0, 4);
        sudokuMap.setCell(5, 0, 6);
        sudokuMap.setCell(8, 0, 7);

        sudokuMap.setCell(6, 1, 4);

        sudokuMap.setCell(1, 2, 1);
        sudokuMap.setCell(6, 2, 6);
        sudokuMap.setCell(7, 2, 5);

        sudokuMap.setCell(0, 3, 5);
        sudokuMap.setCell(2, 3, 9);
        sudokuMap.setCell(4, 3, 3);
        sudokuMap.setCell(6, 3, 7);
        sudokuMap.setCell(7, 3, 8);

        sudokuMap.setCell(4, 4, 7);

        sudokuMap.setCell(1, 5, 4);
        sudokuMap.setCell(2, 5, 8);
        sudokuMap.setCell(4, 5, 2);
        sudokuMap.setCell(6, 5, 1);
        sudokuMap.setCell(8, 5, 3);

        sudokuMap.setCell(1, 6, 5);
        sudokuMap.setCell(2, 6, 2);
        sudokuMap.setCell(7, 6, 9);

        sudokuMap.setCell(2, 7, 1);

        sudokuMap.setCell(0, 8, 3);
        sudokuMap.setCell(3, 8, 9);
        sudokuMap.setCell(5, 8, 2);
        sudokuMap.setCell(8, 8, 5);
    }
    */

    // test map 2
    {
        Map_c::setDimension(3, 3);

        sudokuMap.setCell(0, 0, 8);

        sudokuMap.setCell(2, 1, 3);
        sudokuMap.setCell(3, 1, 6);

        sudokuMap.setCell(1, 2, 7);
        sudokuMap.setCell(4, 2, 9);
        sudokuMap.setCell(6, 2, 2);

        sudokuMap.setCell(1, 3, 5);
        sudokuMap.setCell(5, 3, 7);

        sudokuMap.setCell(4, 4, 4);
        sudokuMap.setCell(5, 4, 5);
        sudokuMap.setCell(6, 4, 7);

        sudokuMap.setCell(3, 5, 1);
        sudokuMap.setCell(7, 5, 3);

        sudokuMap.setCell(2, 6, 1);
        sudokuMap.setCell(7, 6, 6);
        sudokuMap.setCell(8, 6, 8);

        sudokuMap.setCell(2, 7, 8);
        sudokuMap.setCell(3, 7, 5);
        sudokuMap.setCell(7, 7, 1);

        sudokuMap.setCell(1, 8, 9);
        sudokuMap.setCell(6, 8, 4);
    }

    sudokuMap.printMap();

    return true;
}

bool sudoku()
{
    // simply use maximum...
    SudokuCellStackItem_t sudoStack[81];

    if (!initializeSudokuMap(sudoStack[0].map_m))
    {
        return false;
    }


    SudokuCellStackItem_t* pHeader  = NULL;
    int nStackHeader                = 0;
    for (;true;)
    {
        // push into stack
        if (NULL == pHeader)
        {
            //if (NULL != sudoStack[nStackHeader].pMapAvailIter_m)
            //{
            //    delete sudoStack[nStackHeader].pMapAvailIter_m;
            //}

            //sudoStack[nStackHeader].pMapAvailIter_m = new MapAvailableCellIterator_c(sudoStack[nStackHeader].map_m);

            pHeader = &sudoStack[nStackHeader + 1];
            pHeader->map_m = sudoStack[nStackHeader].map_m;

            if (NULL != pHeader->pCellCandidateIter_m)
            {
                delete pHeader->pCellCandidateIter_m;
            }
            pHeader->pCellCandidateIter_m = pHeader->map_m.getCandidateCell(); //sudoStack[nStackHeader].pMapAvailIter_m->next();

            nStackHeader++;
        }
        else
        {
            // refresh current map anyway
            //pHeader = &sudoStack[nStackHeader];
            //pHeader->map_m = sudo[nStackHeader].map_m;

            //pHeader->pCellCandidateIter_m = sudoStack[nStackHeader].pMapAvailIter_m->next();
            //if (NULL == pHeader->pCellCandidateIter_m)

            //nStackHeader++;
        }


        /*
        if (NULL == pHeader->pCellCandidateIter_m)
        {
            // check if all cells are OK

            // Else, that means this solution is run, let's go back and try others
            if (0 <= nStackHeader)
            {
                // if already at the bottom of stack, something must go wrong...
                return false;
            }

            // popup the header, and try next candidate
            nStackHeader--;

            pHeader = &sudoStack[nStackHeader];
        }
        */

        int nCandidate = (NULL != pHeader->pCellCandidateIter_m) ? pHeader->pCellCandidateIter_m->next() : 0;
        if ((0 != nCandidate) &&
                (pHeader->map_m.setCell(pHeader->pCellCandidateIter_m->getX(),
                                        pHeader->pCellCandidateIter_m->getY(),
                                        nCandidate)))
        {
            if (pHeader->map_m.isSolved())
            {
                printf("\n\nFound solution!!!\n");

                // print the result
                pHeader->map_m.printMap();

                // clear the stack

                return true;
            }
            //else
            //{
            //    printf("Try [%d][%d] = %d\n",
            //            pHeader->pCellCandidateIter_m->getX(),
            //            pHeader->pCellCandidateIter_m->getY(),
            //            nCandidate);
            //    // try new one
            //    pHeader->map_m.printMap();
            //}

            // push into stack
            pHeader = NULL;
        }
        else
        {
            if (0 == nCandidate)
            {
                // All possible candidates have been tried, so something has been wrong previous, pop up stack
                nStackHeader --;
            }

            if (nStackHeader <= 0)
            {
                printf("Strange... could not found solution...\n");
                return false;
            }

            // no available digit, this round is failed, popup the header
            pHeader = &sudoStack[nStackHeader];

            // refresh map
            pHeader->map_m = sudoStack[nStackHeader - 1].map_m;

            //printf("Failed, let's fall back and try again...\n\n\n\n\n");
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int main()
{
    printf ("prepare sudoku...\n");

    sudoku();

    return 0;
}
