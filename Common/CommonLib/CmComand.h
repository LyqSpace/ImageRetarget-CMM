#pragma once

class CmComand
{
public:
	static void RunComand(string parameters, bool waiteTillFlished = false);

	static void RunProgram(string fileName, string parameters = "", bool waiteTillFlished = false);
	static void Demo();
};
