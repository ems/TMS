//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//

#ifndef quickSort_H
 
template < class Element, class Cmp >
void qsortCall(Element *array, size_t first, size_t last, Cmp cmp)
{
	const size_t insertionSortBound = 16; /* boundary point to use insertion sort */
	
	register size_t stack_pointer = 0;
	size_t first_stack[128], last_stack[128];
	
	for (;;)
	{
		if (last - first <= insertionSortBound)
		{
			/* for small sort, use insertion sort */
			Element prev_val = array[first];
			for ( register size_t indx = first + 1; indx <= last; ++indx)
			{
				Element cur_val = array[indx];
				if( cmp(cur_val, prev_val) )
				{
					/* out of order */
					array[indx] = prev_val;
					for ( register size_t indx2 = indx - 1; indx2 > first; --indx2)
					{
						if (cmp(cur_val, array[indx2-1]) )
							array[indx2] = array[indx2-1];
						else
							break;
					}
					array[indx2] = cur_val;
				}
				else
				{
					/* in order, advance to next element */
					prev_val = cur_val;
				}
			}
		}
		else
		{
			/* try quick sort */
			register size_t med = (first + last) >> 1;
			/* Choose pivot from first, last, and median position. */
			/* Sort the three elements. */
			if (cmp(array[last], array[first]) )
			{ Element temp = array[first]; array[first] = array[last]; array[last] = temp; }
			if (cmp(array[med], array[first]) )
			{ Element temp = array[med]; array[med] = array[first]; array[first] = temp; }
			if (cmp(array[last], array[med]) )
			{ Element temp = array[last]; array[last] = array[med]; array[med] = temp; }
			
			Element pivot = array[med];
			register size_t up = last, down = first;
			/* First and last element will be loop stopper. */
			/* Split array into two partitions. */
			for (;;)
			{
				do ++down; while (cmp(array[down], pivot) );
				do --up;   while (cmp(pivot, array[up]) );
				
				if (up > down)
				{
					/* interchange L[down] and L[up] */
					Element temp = array[down]; array[down]= array[up]; array[up] = temp;
				}
				else
					break;
			}
			/* stack the partition that is larger */
			if (up - first + 1 >= last - up)
			{
				first_stack[stack_pointer] = first;
				last_stack[stack_pointer++] = up;
				
				first = up + 1;
				/*  tail recursion elimination of
				*  Qsort(array,fun_ptr,up + 1,last)
				*/
			}
			else
			{
				first_stack[stack_pointer] = up + 1;
				last_stack[stack_pointer++] = last;
				
				last = up;
				/* tail recursion elimination of
				* Qsort(array,fun_ptr,first,up)
				*/
			}
			continue;
			/* end of quick sort */
		}
		if (stack_pointer == 0)
			break;

		/* Sort segment from stack. */
		first = first_stack[--stack_pointer];
		last = last_stack[stack_pointer];
  } /* end for */
}
 
template <class Element, class Cmp>
inline void quickSort( Element *array, size_t size, Cmp cmp )
{
	if( size > 0 )
		qsortCall( array, (size_t)0, size-1, cmp );
}

#endif // quickSort_H