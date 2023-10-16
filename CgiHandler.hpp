/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npiya-is <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/08 15:47:50 by npiya-is          #+#    #+#             */
/*   Updated: 2023/09/08 15:50:34 by npiya-is         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>

class CgiHandler
{
public:
	CgiHandler(void);
	CgiHandler(const CgiHandler(&src));
	~CgiHandler(void);
	CgiHandler &operator=(const CgiHandler &src);

private:
	std::string _cgiPath;
};

#endif