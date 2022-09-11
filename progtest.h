#include <stdio.h>
#include <string.h>

typedef char	Carrier[3];		// код авиакомпании
typedef char	FlightNo[5];	// номер рейса
typedef char	Point[4];		// код пункта
typedef long	Fare;				// тариф

// Пункт маршрута
struct RoutePoint {
	RoutePoint	*next;
	Point			point;
	
	RoutePoint() : next( 0 ) {}
};

// Маршрут
class Route {
	RoutePoint	*first;
	
 public:
	Route();
	~Route();
	
	// чтение из файла
	int read( const char *fileName );	// 0 - OK, !=0 - ошибка
	
	// проверка маршрута на:
	//   несовпадение соседних пунктов
	//   не менее двух пунктов в маршруте
	int check() const;	// 0 - OK, !=0 - ошибка
	
	// итератор
	RoutePoint	*iterator( RoutePoint *& iter ) const;
	
	// печать на stdout
	void print( const char *prefix ) const;
};

// Рейс
struct Flight {
	Carrier	carrier;		// перевозчик
	FlightNo	flightNo;	// номер рейса
	Point		depPoint;	// пункт отправления
	Point		arrPoint;	// пункт назначения
	Fare		fare;			// тариф
	
	void	print() const;
};

//
class ScheduleItem : public Flight {
	friend class Schedule;

   ScheduleItem	*next;

 public:
	ScheduleItem() : next( 0 ) {}
};

// Расписание
class Schedule {
	ScheduleItem	*firstFlight;
 public:
	Schedule();
	~Schedule();
	
	// чтение из файла
	int read( const char *fileName );	// 0 - OK, !=0 - ошибка
	
	// итератор
	ScheduleItem	*iterator( ScheduleItem *& iter ) const;
	
	// печать на stdout
	void	print() const;

   // Формирует вырезку из расписания только между заданными пунктами.
   bool GetFlights(Point depPoint, Point arrPoint, Schedule& schedule) const;
};

// Участок перевозки
struct TransLeg {
	TransLeg	*next;
	Flight	flight;
	
	TransLeg() : next ( 0 ) {}
};

// Перевозка
class Transportation 
{
	TransLeg	*firstLeg;
 public:
	Transportation();
	~Transportation();
	
	void	flush();
	int	buildCheapest( const Route & route, const Schedule & schedule );
	void	print() const;

   int Length() const;
   bool IsUnique() const;
   int TotalFare() const;
	
 private:
	static Flight	*findCheapestFlight( const Schedule & schedule, const char *depPoint, const char *arrPoint, Carrier mainCarrier );

   /// Рассчитывает маршрут с уникальным перевозчиком (если есть).
   static Flight	*findMainCarrierFlight( const Schedule & schedule, const char *depPoint, const char *arrPoint, Carrier mainCarrier );
};