#include "Header.h"

int main(int argc, char *argv[])
{
	Header *m_header = new Header();
	m_header->Init();
	m_header->Run();
	m_header = NULL;
	delete[] m_header;
	return 0;
}


