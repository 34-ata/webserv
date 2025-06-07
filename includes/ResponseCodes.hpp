#ifndef RESPONSE_CODES_HPP
#define RESPONSE_CODES_HPP

enum ResponseCodes
{
	OK			   = 200,
	CREATED		   = 201,
	BAD_REQ		   = 400,
	NOT_FOUND	   = 404,
	METH_NOT_ALLOW = 405,
	INT_SERV_ERR   = 500,
};

#endif
