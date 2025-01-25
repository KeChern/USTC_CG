//#include "DArray.h"
//#include <iostream>
//#include <assert.h>
//
// //default constructor
//DArray::DArray()
//{
//	Init();
//}
//
// //set an array with default values
//DArray::DArray(int nSize, double dValue)
//	:m_pData(new double[static_cast<size_t>(1.5 * nSize) + 1]), m_nSize(nSize), m_nMax(static_cast<size_t>(1.5 * nSize) + 1)
//{
//	for (int i = 0; i < nSize; i++)
//	{
//		m_pData[i] = dValue;
//	}
//	for (int i = nSize; i < m_nMax; i++)
//	{
//		m_pData[i] = 0;
//	}
//}
//
//DArray::DArray(const DArray& arr) 
//	:m_pData(new double[arr.m_nMax]),m_nSize(arr.m_nSize),m_nMax(arr.m_nMax)
//{
//	for (int i = 0; i < m_nMax; i++)
//	{
//		m_pData[i] = arr.m_pData[i];
//	}
//}
//
//// deconstructor
//DArray::~DArray() {
//	Free();
//}
//
//// display the elements of the array
//void DArray::Print() const 
//{
//	std::cout << "size=" << m_nSize << ":" << std::endl;
//	for (int i = 0; i < m_nSize; i++)
//	{
//		std::cout << m_pData[i] << " ";
//	}
//	std::cout << std::endl;
//}
//
//// initilize the array
//void DArray::Init() 
//{
//	m_pData = nullptr;
//	m_nSize = 0;
//	m_nMax = 0;
//}
//
//// free the array
//void DArray::Free() 
//{
//	delete[] m_pData;
//	m_pData = nullptr;
//	m_nSize = 0;
//	m_nMax = 0;
//}
//
//// get the size of the array
//int DArray::GetSize() const 
//{
//	return m_nSize;
//}
//
//// set the size of the array
//void DArray::SetSize(int nSize)
//{
//	if (nSize == 0)
//	{
//		Init();
//	}
//	else if (m_nSize == 0)
//	{
//		m_nSize = nSize;
//		m_nMax = static_cast<size_t>(1.5 * nSize) + 1;
//		m_pData = new double[m_nMax];
//	}
//	else if (nSize <= m_nMax)
//	{
//		m_nSize = nSize;
//	}
//	else
//	{
//		m_nMax = static_cast<size_t>(1.5 * nSize) + 1;
//		double* pData = new double[m_nMax];
//		for (int i = 0; i < m_nSize; i++)
//		{
//			pData[i] = m_pData[i];
//		}
//		for (int i = m_nSize; i < m_nMax; i++)
//		{
//			pData[i] = 0;
//		}
//		m_nSize = nSize;
//		delete[] m_pData;
//		m_pData = pData;
//	}
//}
//
//// get an element at an index
//const double& DArray::GetAt(int nIndex) const
//{
//	assert(nIndex >= 0 && nIndex < m_nSize);
//	return m_pData[nIndex];
//}
//
//// set the value of an element 
//void DArray::SetAt(int nIndex, double dValue)
//{
//	assert(nIndex >= 0 && nIndex < m_nSize);
//	m_pData[nIndex] = dValue;
//}
//
//// overload operator '[]'
//double& DArray::operator[](int nIndex)
//{
//	assert(nIndex >= 0 && nIndex < m_nSize);
//	return m_pData[nIndex];
//}
//
//// overload operator '[]'
//const double& DArray::operator[](int nIndex) const
//{
//	assert(nIndex >= 0 && nIndex < m_nSize);
//	return m_pData[nIndex];
//}
//
//// add a new element at the end of the array
//void DArray::PushBack(double dValue)
//{
//	if (m_nSize < m_nMax)
//	{
//		m_pData[m_nSize] = dValue;
//		m_nSize++;
//	}
//	else
//	{
//		m_nSize++;
//		m_nMax = static_cast<size_t>(1.5 * m_nSize) + 1;
//		double* pData = new double[m_nMax];
//		for (int i = 0; i < m_nSize - 1; i++)
//		{
//			pData[i] = m_pData[i];
//		}
//		pData[m_nSize - 1] = dValue;
//		for (int i = m_nSize; i < m_nMax; i++)
//		{
//			pData[i] = 0;
//		}
//		delete[] m_pData;
//		m_pData = pData;
//	}
//}
//
//// delete an element at some index
//void DArray::DeleteAt(int nIndex)
//{
//	assert(nIndex >= 0 && nIndex < m_nSize);
//	for (int i = nIndex; i < m_nSize - 1; i++)
//	{
//		m_pData[i] = m_pData[i + 1];
//	}
//	m_nSize--;
//}
//
//// insert a new element at some index
//void DArray::InsertAt(int nIndex, double dValue)
//{
//	assert(nIndex >= 0 && nIndex <= m_nSize);
//	if (m_nSize < m_nMax)
//	{
//		for (int i = m_nSize; i > nIndex; i--)
//		{
//			m_pData[i] = m_pData[i - 1];
//		}
//		m_pData[nIndex] = dValue;
//		m_nSize++;
//	}
//	else
//	{
//		m_nSize++;
//		m_nMax = static_cast<size_t>(1.5 * m_nSize) + 1;
//		double* pData = new double[m_nMax];
//		for (int i = 0; i < nIndex; i++)
//		{
//			pData[i] = m_pData[i];
//		}
//		pData[nIndex] = dValue;
//		for (int i = nIndex+1; i < m_nSize; i++)
//		{
//			pData[i] = m_pData[i - 1];
//		}
//		delete[] m_pData;
//		m_pData = pData;
//	}
//}
//
//// overload operator '='
//DArray& DArray::operator = (const DArray& arr)
//{
//	delete[] m_pData;
//	m_nSize = arr.m_nSize;
//	m_nMax = static_cast<size_t>(1.5 * m_nSize) + 1;
//	m_pData = new double[m_nMax];
//	for (int i = 0; i < m_nSize; i++)
//	{
//		m_pData[i] = arr[i];
//	}
//	return *this;
//}
//
//void DArray::Reserve(int nSize)
//{
//	assert(nSize > 0);
//	if (nSize <= m_nMax)
//	{
//		return;
//	}
//	while (m_nMax < nSize)
//	{
//		m_nMax = m_nMax == 0 ? 1 : 2 * m_nMax;
//	}
//	double* pData = new double[m_nMax];
//	memcpy(pData, m_pData, m_nSize * sizeof(double));
//	delete[] m_pData;
//	m_pData = pData;
//}