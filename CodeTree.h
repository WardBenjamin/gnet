/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */
#pragma once

#include <string>
#include <vector>
#include <map>

struct CodeTree {
	static CodeTree* Create(const std::map<std::string, int>& histogram);

	std::string value;
	std::vector<CodeTree*> child;
};
