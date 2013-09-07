#ifndef _SIMPLE_VECTOR_H_
#define _SIMPLE_VECTOR_H_

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// Very Simple dynamic array
//
//  Try to keep it as simple as possible, if it can not meet the requirement,
//  the recommended solution is to use other implmenetaion rather than add new
//  functionality
//
////////////////////////////////////////////////////////////////////////////////
template <class T> class SimpleVector
{
    public:

        inline SimpleVector (int initAlloc)
        {
            this->num   = 0;
            this->numAlloc = initAlloc;
            void* temp  = malloc(sizeof (T) * this->numAlloc);
            assert(temp);
            this->array = (T*)temp;
            //this->resize();
        }

        inline SimpleVector ()
        {
            this->num       = 0;
            this->numAlloc  = 0;
            this->array     = NULL;
        }


        inline ~SimpleVector ()
        {
            if (this->array)
            {
                free(this->array);
            }
        }

        //////////////////////////////////////////////////////////////////////////////// 
        // Return the number of elements put into this vector.
        //////////////////////////////////////////////////////////////////////////////// 
        inline int size() const { return this->num; }
        inline T* getArray()    { return array; }

        // push of stack
        inline void append(T t)
        {
            // if all items occupied, re-allocate the buffer
            if (this->num >= this->numAlloc)
            {
                setSize(this->num + 1);
            }
            else
            {
                this->num++;
            }

            this->array[this->num - 1] = t;
        }

        //inline T takeAt(int i)
        //{
        //    assert (i >= 0 && this->num - i > 0);
        //    T pRet = this->array[i];

        //    this->num--;
        //    return this->array[this->num];
        //}

        // pop of stack
        inline T takeLast()
        {
            assert (this->num > 0);

            this->num--;
            return this->array[this->num];
        }

        // get last item in the list
        inline T getLast()
        {
            assert (this->num > 0);

            return this->array[this->num - 1];
        }
        ///////////////////////////////////////////////////////////////
        // NOTE: 
        //  it's not responsible for releasing object, and only reset 
        //  the number of item to zero.
        //
        //  Please release the object outside if needed
        ///////////////////////////////////////////////////////////////
        inline void clear()
        {
            this->num = 0;
        }

        ///**
        // * \brief Increase the vector size by one.
        // *
        // * May be necessary before calling misc::SimpleVector::set.
        // */
        //inline void increase() { setSize(this->num + 1); }

        //////////////////////////////////////////////////////////////////////////////// 
        // Set the size explicitely.
        // 
        // May be necessary before calling SimpleVector::set.
        //////////////////////////////////////////////////////////////////////////////// 
        inline void setSize(int newSize)
        {
            assert (newSize >= 0);
            this->num = newSize;
            this->resize ();
        }

        ////////////////////////////////////////////////////////////////////////////////  
        // Return the reference of one element.
        ////////////////////////////////////////////////////////////////////////////////  
        inline T* getRef (int i) {
            assert (i >= 0 && this->num - i > 0);
            return array + i;
        }

        //////////////////////////////////////////////////////////////////////////////// 
        // Return the one element, explicitety.
        // 
        //  The element is copied, so for complex elements, you should rather used
        //  SimpleVector::getRef.
        //////////////////////////////////////////////////////////////////////////////// 
        inline T get (int i)
        {
            assert (i >= 0 && this->num - i > 0);
            return this->array[i];
        }

        inline const T get(int i) const
        {
            return ((SimpleVector<T>*)this)->get(i);
        }

        //////////////////////////////////////////////////////////////////////////////// 
        // Store an object in the vector.
        //////////////////////////////////////////////////////////////////////////////// 
        inline void set (int i, T t)
        {
            assert (i >= 0 && this->num - i > 0);
            this->array[i] = t;
        }

        /*
        void deleteAllObject()
        {
            int i;
            for(i = 0 ; i < num ; ++ i)
            {
                delete array[i];
            }
            num = 0 ;
        }
        */

    protected:
        virtual void resize ()
        {
            if (array == NULL)
            {
                this->numAlloc = 1;
                this->array = (T*) malloc (sizeof (T));
            }

            if (this->numAlloc < this->num)
            {
                //int sOldNumAlloc = this->numAlloc;
                this->numAlloc = (this->num < 100) ?  this->num : this->num + this->num/10;
                this->array = (T*) realloc(this->array, (this->numAlloc * sizeof (T))/*,(sOldNumAlloc * sizeof (T) )*/);
            }
            assert(NULL != this->array);
        }

    protected:
        T *array;
        int num;
        int numAlloc;

};

#endif
