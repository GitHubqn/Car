#include<Windows.h>
#include <iostream>
#include <conio.h>
#include<thread>

using std::cin;
using std::cout;
using std::endl;

#define MIN_TANK_VOLUME 20
#define MAX_TANK_VOLUME 150

#define Escape 27
#define Enter  13

#define Up 72
#define Down 80
#define Space  32


class Tank
{
	const int VOLUME;
	double fuel_level;
public:
	int get_VOLUME()const {
		return VOLUME;
	}
	double get_fuel_level()const {
		return fuel_level;
	}
	void fill(double amount) {
		if (amount < 0)return;
		fuel_level += amount;
		if (fuel_level > VOLUME)fuel_level = VOLUME;
	}
	double give_fuel(double amount) {
		fuel_level -= amount;
		if (fuel_level < 0) { fuel_level = 0; }
		return fuel_level;
	}
	Tank(int volume) :
		VOLUME(
			volume < MIN_TANK_VOLUME ? MIN_TANK_VOLUME :
			volume > MAX_TANK_VOLUME ? MAX_TANK_VOLUME :
			volume
		) 
	{
		fuel_level = 0;
		cout << "Tank is ready " << this << endl;
	}
	~Tank() {
		cout << "Tank is over " << this << endl;
	}
	void info()const {
		cout << "Volume: " << VOLUME << " liters;\n";
		cout << "Fuel level: " << get_fuel_level() << " liters;\n";

	}
};

#define MIN_ENGINE_CONSUMPTION 3
#define MAX_ENGINE_CONSUMPTION 30

class Engine 
{
	const double CONSUMPTION;
	double consumption_per_sec;
	bool is_started;

public:
	double get_CONSUMPTION()const {
		return CONSUMPTION;
	}
	double get_consumption_per_sec()const {
		return consumption_per_sec;
	}
	void set_consumption_per_sec() {
		consumption_per_sec = CONSUMPTION * 3e-5;
	}
	void start() {
		is_started = true;
	}
	void stop() {
		is_started = false;
	}
	bool started() const {
		return is_started;
	}

	

	Engine(double consumption) :CONSUMPTION
	(
		consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		consumption
	)
	{
		set_consumption_per_sec();
		is_started = false;
		cout << "Engine is ready: " << this << endl;
	}
	~Engine() {
		cout << "Engine is over: " << this << endl;
	}

	void info()const {
		cout << "Conusumption: \n";
		cout << CONSUMPTION << " liters/100km\n";
		cout << consumption_per_sec << " liters/second\n";
	}
};

#define MAX_SPEED_LOW_LIMIT  120
#define MAX_SPEED_HIGT_LIMIT 400

class Car
{
	Engine engine;
	Tank tank;
	int speed;
	const int MAX_SPEED;
	bool driver_inside;
	struct 
	{
		std::thread panel_thread;
		std::thread consumption_thread;
		std::thread slowdown_thread;
	} control_threads;
public:
	
	Car(int max_speed, double consumption, int volume) :engine(consumption), tank(volume),
		MAX_SPEED
		(
			max_speed < MAX_SPEED_LOW_LIMIT ? MAX_SPEED_LOW_LIMIT :
			max_speed < MAX_SPEED_HIGT_LIMIT ? MAX_SPEED_HIGT_LIMIT :
			max_speed
		)
	{
		speed = 0;
		driver_inside = false;
		cout << "Машина, " << this << " - готова, нажмите Enter что бы сесть в неё" << endl;
	}
	~Car() 
	{
		cout << "Car is over" << this << endl;
	}

	void info()const
	{
		engine.info();
		tank.info();
		cout << "Max speed: " << MAX_SPEED << " km/h\n";
	}

	int get_speed() const {
		return speed;
	}

	void get_in()
	{
		driver_inside = true;
		control_threads.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		driver_inside = false;
		if (control_threads.panel_thread.joinable()) {
			control_threads.panel_thread.join();
		}
		system("CLS");
		cout << "Вы вышли из машины" << endl;
	}

	void panel() {
		while (driver_inside)
		{
			system("CLS");
			cout << "Speed: " << speed << " km/h\n";
			cout << "Engine is: " << (engine.started() ? "started " : "stopped ") << endl;
			cout << "Fuel level: " << tank.get_fuel_level() << " liters" << endl;
			cout << "Consumption: " << engine.get_consumption_per_sec() << " liters/second";
			Sleep(250);
		}
	}

	void spend_fuel() {
		while (engine.started())
		{
			tank.give_fuel(engine.get_consumption_per_sec());
			Sleep(1000);
		}
	}

	void control()
	{
		char key;
		control_threads.slowdown_thread = std::thread(&Car::slowdown, this);
		do
		{
			key = _getch();
			switch (key)
			{
			case Enter:
				if (driver_inside and speed == 0)
				{
					get_out();
				}
				else if (speed == 0) {
					get_in();
				}
				break;
			case Space:
				if (engine.started() and speed == 0)
				{
					stop_engine();
				}
				else if (speed == 0)
				{
					start_engine();
				}
				break;
			case Escape:
				stop_engine();
				get_out();
				break;
			case Up:
				if (driver_inside and engine.started())
				{
					speed_up(10);
				}
				break;
			case Down:
				if (driver_inside and engine.started())
				{
					speed_down(10);
				}
				break;
			
			default:
				break;
			}
		} while (key != Escape);
		if (control_threads.slowdown_thread.joinable())
		{
			control_threads.slowdown_thread.join();
		}
	}
	
	void refuel(double amount) {
		tank.fill(amount);
	}

	void consume_fuel() {
		while (engine.started())
		{
			if (tank.give_fuel(engine.get_consumption_per_sec()) <= 0)
			{
				engine.stop();
				break;
			}
			Sleep(500);
		}
	}

	void start_engine() {
		if (tank.get_fuel_level() > 0)
		{
			engine.start(); 
			control_threads.consumption_thread = std::thread(&Car::consume_fuel, this);
		}
	}

	void stop_engine() {
		engine.stop();
		if (control_threads.consumption_thread.joinable())
		{
			control_threads.consumption_thread.join();
		}
	}

	void speed_up(int amount) {
		amount < 0 ? amount = 0 : amount > 10 ? amount = 10 : amount;

		speed += amount;
		speed > MAX_SPEED ? speed = MAX_SPEED : speed;
	}

	void speed_down(int amount) {
		amount < 0 ? amount = 0 : amount > 10 ? amount = 10 : amount;

		speed -= amount;

		speed < 0 ? speed = 0 : speed;
	}

	void slowdown() {
		while (true)
		{
			if (!driver_inside)
			{
				break;
			}
			speed_down(1);
			Sleep(4000);
		}
	}
};


//#define TANK_CHECK

void main()
{
    setlocale(LC_ALL, "");
	Car bmw(250, 10, 80);
	bmw.refuel(80);
	bmw.control();

#if defined TANK_CHECK
    Tank tank(3000);
	do
	{
		tank.info();
		cout << "Введите объем топлива: ";
		cin >> fuel;

		tank.fill(fuel);
	} while (fuel);
#endif 

	
}

