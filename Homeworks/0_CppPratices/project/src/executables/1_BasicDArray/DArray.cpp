// implementation of class DArray
#include "DArray.h"
#include<iostream>
#include<assert.h>
using namespace std;
// default constructor
DArray::DArray() {
	Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue) {
	//TODO
	m_nSize = nSize;
	m_pData = new double[nSize];
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = dValue;
	}
}

DArray::DArray(const DArray& arr) {
	//TODO
	if (arr.m_nSize == 0)
	{
		Init();
	}
	else
	{
		m_nSize = arr.m_nSize;
		m_pData = new double[m_nSize];
		for (int i = 0; i < m_nSize; i++)
		{
			m_pData[i] = arr.m_pData[i];
		}
	}
}

// deconstructor
DArray::~DArray() {
	Free();
}

// display the elements of the array
void DArray::Print() const {
	//TODO
	if (m_nSize == 0)
	{
		cout << "Êý×éÎª¿Õ£¡" << endl;
	}
	else
	{
		for (int i = 0; i < m_nSize; i++)
		{
			cout << m_pData[i] << " ";
		}
		cout << endl;
	}
}

// initilize the array
void DArray::Init() {
	//TODO
	m_nSize = 0;
	m_pData = nullptr;
}

// free the array
void DArray::Free() {
	//TODO
	m_nSize = 0;
	delete[]m_pData;
}

// get the size of the array
int DArray::GetSize() const {
	//TODO
	return m_nSize;
	//return 0; // you should return a correct value
}

// set the size of the array
void DArray::SetSize(int nSize) {
	//TODO
	if (nSize == 0)
	{
		Init();
	}
	else if (m_nSize == 0)
	{
		double* pData = new double[nSize];
		for (int i = 0; i < nSize; i++)
		{
			pData[i] = 0;
		}
		m_nSize = nSize;
		m_pData = pData;
	}
	else
	{
		if (m_nSize == nSize)
		{
			return;
		}
		else if (m_nSize < nSize)
		{
			double* pData = new double[nSize];
			for (int i = 0; i < m_nSize; i++)
			{
				pData[i] = m_pData[i];
			}
			for (int i = m_nSize; i < nSize; i++)
			{
				pData[i] = 0;
			}
			m_nSize = nSize;
			m_pData = pData;
		}
		else
		{
			double* pData = new double[nSize];
			for (int i = 0; i < nSize; i++)
			{
				pData[i] = m_pData[i];
			}
			m_nSize = nSize;
			m_pData = pData;
		}
	}
}

// get an element at an index
const double& DArray::GetAt(int nIndex) const {
	//TODO
	return m_pData[nIndex];
	//static double ERROR; // you should delete this line
	//return ERROR; // you should return a correct value
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) {
	//TODO
	m_pData[nIndex] = dValue;
}

// overload operator '[]'
double& DArray::operator[](int nIndex){
	//TODO
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
	//static double ERROR; // you should delete this line
	//return ERROR; // you should return a correct value
}

// overload operator '[]' c
// const
const double& DArray::operator[](int nIndex) const{
	assert(nIndex >= 0 && nIndex < m_nSize);
	return m_pData[nIndex];
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) {
	//TODO
	this->SetSize(this->m_nSize + 1);
	SetAt(this->m_nSize - 1, dValue);
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	//TODO
	for (int i = nIndex; i < m_nSize - 1; i++)
	{
		(*this)[i] = (*this)[i + 1];
	}
	SetSize(this->m_nSize - 1);
}

// insert a new element at some index
void DArray::InsertAt(int nIndex, double dValue) {
	//TODO
	SetSize(this->m_nSize + 1);
	for (int i = m_nSize - 1; i > nIndex; i--)
	{
		(*this)[i] = (*this)[i - 1];
	}
	(*this)[nIndex] = dValue;
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	//TODO
	m_nSize = arr.m_nSize;
	m_pData = new double[m_nSize];
	for (int i = 0; i < m_nSize; i++)
	{
		m_pData[i] = arr[i];
	}
	return *this;
}
