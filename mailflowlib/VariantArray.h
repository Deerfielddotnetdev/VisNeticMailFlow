/***************************************************************************\
||             
||  $Header: /root/MAILSTREAMLIBRARY/VariantArray.h,v 1.1 2005/05/06 14:56:44 markm Exp $
||
||  Copyright © 2001 Deerfield.com, all rights reserved
||                                         
||  COMMENTS:   Converts custom classes to 2-dimensional arrays of variants.
||              
\\*************************************************************************/

// only include this file once
#ifndef VARIANT_ARRAY_H
#define VARIANT_ARRAY_H


////////////////////////////////////////////////////////////////////////////////
// This function object template is used to convert custom classes
// to 2-dimensional arrays of variants.
//
// It requires the custom class to implement the public member function:
//
//	void ConvertToVariant( int col, VARIANT* pVar )
//	{
//		switch( col )
//		{
//			...		
//		}
//	}
//
////////////////////////////////////////////////////////////////////////////////
template <class T>
class CVariantArray
{
public:

	// default constructor
	CVariantArray() : m_parray(NULL),m_i(0)
	{
	}

	// destructor
	~CVariantArray()
	{
		if( m_parray )
		{
			SafeArrayUnaccessData( m_parray->parray );
			m_parray = NULL;
		}
	}

	// Create function - call this to allocate the array before walking the list
	HRESULT Create( int rows, int cols, VARIANT* parray )
	{
		HRESULT hresult;
		
		// check for zero rows
		if( rows == 0)
		{
			parray->vt = VT_EMPTY;
			return S_OK;
		}
		
		m_cols = cols;
		m_rows = rows;
		m_parray = parray;

		//  bounds format: {{#rows,first row index},{#cols, first col index}}
		SAFEARRAYBOUND bounds[2] = {{rows,1},{cols,1}};

		// Create the output array
		m_parray->parray = SafeArrayCreate( VT_VARIANT, 2, bounds );

		if( m_parray->parray == NULL )
			return E_OUTOFMEMORY;

		hresult = SafeArrayAccessData( m_parray->parray, (void**) &m_pVar);
		if( SUCCEEDED( hresult) )
		{
			m_parray->vt = VT_ARRAY | VT_VARIANT;
		}
		return hresult;
	}

	// The magic part
	void operator() ( T& value ) 
	{
		int i;
		for ( i = 0; i < m_cols; i++ )
		{
			value.ConvertToVariant( i, &(m_pVar[m_i + i*m_rows]) );
		}
		m_i++;
	}

protected:
	int m_i;
	int m_rows;
	int m_cols;
	VARIANT* m_parray;
	VARIANT* m_pVar;
};


/*---------------------------------------------------------------------------\             
||  Matthew McDermott
||           
||  Comments:	Helper Macros for ConvertToVariant
\*--------------------------------------------------------------------------*/
#define VARIANT_ARRAY_STRING(a) \
	pVar->vt = VT_BSTR; pVar->bstrVal = SysAllocString(a); \
	if ((pVar->bstrVal == NULL) && (wcslen(a) > 0)) \
	{\
		THROW_EMS_EXCEPTION(E_MemoryError, CEMSString(EMS_STRING_ERROR_OUTPUT_ARRAY));\
	}\

#define VARIANT_ARRAY_INT(a) pVar->vt = VT_I4; pVar->lVal = a;
#define VARIANT_ARRAY_BOOL(a) pVar->vt = VT_BOOL; pVar->boolVal = (a) ? 0xffff : 0;
#define VARIANT_ARRAY_DATE(a) pVar->vt = VT_DATE; TimeStampToVarDate( a, pVar->date );

#define CREATE_VARIANT_ARRAY(a, b, c) \
	CVariantArray<a> converter;\
	if ((converter.Create(b.size(), c, array )) != S_OK) \
	{ THROW_EMS_EXCEPTION(E_MemoryError, CEMSString(EMS_STRING_ERROR_OUTPUT_ARRAY)); } \
	for_each( b.begin(), b.end(), converter );

#endif // VARIANT_ARRAY_H