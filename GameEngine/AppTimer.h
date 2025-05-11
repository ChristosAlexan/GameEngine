#pragma once

#include <iostream>
#include <chrono>

class AppTimer
{
public:

	AppTimer();
	double GetMilisecondsElapsed();
	int GetSecondsElapsed();
	void Restart();
	bool Stop();
	bool Start();
	void StartSeconds();
	float GetMilliseconds();

	void CalculateDeltaTime(float& deltaTime, float& fps);

private:

	bool isRunning = false;
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> stop;

private:
	std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
};
