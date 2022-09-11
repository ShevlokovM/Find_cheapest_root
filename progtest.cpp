#include <stdio.h>
#include <string.h>

//-------- new start ------------//
#include <set>
#include <vector>
#include <thread>
#include <mutex>

//--------  new end  ------------//

typedef char	Carrier[3];		// код авиакомпании
typedef char	FlightNo[5];	// номер рейса
typedef char	Point[8];		// код пункта
typedef /*new*/double	Fare;				// тариф

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
	bool withDiscount = false; // new если со скидкой, будет true
	
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
	
};

// Участок перевозки
struct TransLeg {
	TransLeg	*next;
	Flight	flight;
	//-------- new start ------------//
	// добавлено поле  - общая стоиость перелета по маршруту
	// заполняется только в первом ноде списка
	Fare commonRootPrice;
	//--------  new end  ------------//
	
	
	TransLeg() : next ( 0 ) 
	//-------- new start ------------//
	, commonRootPrice ( 0 ) {}
	
	//--------  new end  ------------//
};

// Перевозка
class Transportation {
	TransLeg	*firstLeg;
 public:
	Transportation();
	~Transportation();

	void	flush();
	int	buildCheapest( const Route & route, const Schedule & schedule );
	void	print() const;
	
 private:
	Flight	*findCheapestFlight( const Schedule & schedule, const char *depPoint, const char *arrPoint, /*new start*/std::set<Carrier*>* carrierList = nullptr/*new end*/);
	//-------- new start ------------//
	// функция ищет маршруты только одной авиакомпании
	Flight* findCheapestOneCarrierFlight(const Schedule& schedule, const char* depPoint, const char* arrPoint, const char* carrier);
	//--------  new end  ------------//
};

//___ Реализация _________________________________

//___ Route ______________________________________

Route::Route()
	: first( 0 )
{
}

Route::~Route()
{
	for( RoutePoint *item = first; item; ) {
		RoutePoint	*toDelete = item;
		item = item->next;
		delete toDelete;
	}
}

int Route::read( const char *fileName )
{
	RoutePoint	*lastItem = 0;

	FILE	*f = fopen( fileName, "r" );
	if( !f ) return 1;
	
	Point	readPoint;
	while( fscanf( f, "%3s", readPoint ) == 1 ) {
		RoutePoint	*newItem = new RoutePoint;
		strcpy( newItem->point, readPoint );
		if( lastItem ) {
			lastItem->next = newItem;
		} else
			first = newItem;
		lastItem = newItem;
	}
	
	fclose( f );
	return 0;
}

int Route::check() const
{
	if( !first || !first->next )
		return 1;
		
	RoutePoint	*iter = 0;
	while( iterator( iter )) {
		if( iter->next && 0==strcmp( iter->point, iter->next->point ))
			return 1;
	}
	return 0;
}

RoutePoint * Route::iterator( RoutePoint *& iter ) const
{
	if( iter )
		iter = iter->next;
	else
		iter = first;
	return iter;
}

void Route::print( const char *prefix ) const
{
	if( prefix ) 
		printf( prefix );
		
	RoutePoint	*iter = 0;
	while( iterator( iter )) {
		printf( "%s ", iter->point );
	}
	
	printf( "\n" );
}

//___ Расписание ___________________________________________

void Flight::print() const
{
	printf( "%-2s %-4s %-3s %-3s %.2f", //new переделано под double
		carrier, 
		flightNo, 
		depPoint,
		arrPoint,
		(withDiscount ? fare * 0.8 : fare) ); // new вывод стоимости со скидкой или без
}

Schedule::Schedule()
	: firstFlight( 0 )
{
}

Schedule::~Schedule()
{
	for( ScheduleItem *flight = firstFlight; flight; ) {
		ScheduleItem	*toDelete = flight;
		flight = flight->next;
		delete toDelete;
	}
}

int Schedule::read( const char *fileName )
{
	ScheduleItem	*lastFlight = 0;

	FILE	*f = fopen( fileName, "r" );
	if( !f ) return 1;

	Flight	fl;
	while( fscanf( f, "%2s %4s %3s %3s %lf", fl.carrier, fl.flightNo, fl.depPoint, fl.arrPoint, &fl.fare ) == 5 ) { //new
		ScheduleItem	*newFlight = new ScheduleItem;
		*(Flight*)newFlight = fl;
		if( lastFlight ) {
			lastFlight->next = newFlight;
		} else
			firstFlight = newFlight;
		lastFlight = newFlight;
	}

	fclose( f );
	return 0;
}

ScheduleItem * Schedule::iterator( ScheduleItem *& iter ) const
{
	if( iter )
		iter = iter->next;
	else
		iter = firstFlight;
	return iter;
}

void Schedule::print() const
{
	ScheduleItem	*f = 0;
	while( iterator( f )) {
		f->print();
		printf( "\n" );
	}
}

//___ Transportation ______________________________________________

Transportation::Transportation()
	: firstLeg( 0 )
{
}

Transportation::~Transportation()
{
	flush();
}

void Transportation::flush()
{
	for( TransLeg *leg = firstLeg; leg; ) {
		TransLeg	*toDelete = leg;
		leg = leg->next;
		delete toDelete;
	}
	firstLeg = 0;
}

// добавлен элемент по умолчанию, список возможных авиакомпаний, по которому можно дальше производить поиск
Flight * Transportation::findCheapestFlight( const Schedule & schedule, const char *depPoint, const char *arrPoint
	/*new start*/, std::set<Carrier*>* carrierList/*new end*/)
{
	Flight	*flightWithMinimalFare = 0;
	
	ScheduleItem	*schedItem = 0;
	while( schedule.iterator( schedItem )) {
		if(	0 != strcmp( schedItem->depPoint, depPoint ) ||
				0 != strcmp( schedItem->arrPoint, arrPoint )) continue;	
		//-------- new start ------------//
		// фрагмент кода создает список всех авиакомпаний, у которых есть рейсы из первой точки во вторую
		// на основании этого списка далее будет происходить поиск маршрутов в привязке к компании
		// 
		else if (carrierList) {			
			if(!carrierList->count(&schedItem->carrier))
				carrierList->insert(&schedItem->carrier);
		}
		//--------  new end  ------------//
		if (!flightWithMinimalFare || flightWithMinimalFare->fare > schedItem->fare) {
			flightWithMinimalFare = schedItem;
		}
	} 
	
	return flightWithMinimalFare;
}

//-------- new start ------------//
// функция ищет самые дешевые маршруты конкретной авиакомпании
Flight* Transportation::findCheapestOneCarrierFlight(const Schedule& schedule, const char* depPoint, const char* arrPoint
	, const char* carrier)
{
	Flight* flightWithMinimalFare = 0;

	ScheduleItem* schedItem = 0;
	while (schedule.iterator(schedItem)) {
		if (0 != strcmp(schedItem->depPoint, depPoint) ||
			0 != strcmp(schedItem->arrPoint, arrPoint) ||
			0 != strcmp(schedItem->carrier, carrier)) continue;	// тут условие по соответствию авиакомпании	
		
		if (!flightWithMinimalFare || flightWithMinimalFare->fare > schedItem->fare) {
			flightWithMinimalFare = schedItem;
		}
	}

	return flightWithMinimalFare;
}

// функция удаляет все элементу связанного списка, кроме первого
template<typename T>
void deleteList(T ptr)
{
	if (ptr->next)
		deleteList(ptr->next);
	else return;

	delete ptr->next;
}

//--------  new end  ------------//

int Transportation::buildCheapest( const Route & route, const Schedule & schedule )
{
	flush();
	
	TransLeg	*lastLeg = 0;
	std::set<Carrier*> carrierList;

	RoutePoint	*routePoint = 0;
	while( route.iterator( routePoint ) && routePoint->next ) {
		Flight* cheapestFlight;
		//-------- new start ------------//
		// нужно заполнить список возможных авиакомпаний
		// чтобы попасть в этот списко, у авиакомпании должен быть хотя бы один рейс из первого пункта маршрута во второй
		if (carrierList.size() == 0) {
			// поэтому, если список пустой, а он по-идее пустой только на первой итерации,
			// то вызываем фуекцию поиска самого дешевого маршрута с опцией заполнения списка
			cheapestFlight = findCheapestFlight(schedule, routePoint->point, routePoint->next->point, &carrierList);
		}
		else {
			cheapestFlight = findCheapestFlight(schedule, routePoint->point, routePoint->next->point);
		}
		
		if (!cheapestFlight) {
			// удаляем связанный список, т.к. у маршрута нет продолжения, и он не дойдет до последней точки
			deleteList(firstLeg);
			delete firstLeg;
			firstLeg = 0;
			break; // пока просто прерываем цикл, нужно будет еще отработать маршруты с одной авиакомпанией
		}
		//--------  new end  ------------//

		TransLeg	*newLeg = new TransLeg;
		newLeg->flight = *cheapestFlight;
		

		if( lastLeg ) 
			lastLeg->next = newLeg;
		else
			firstLeg = newLeg;

		//-------- new start ------------//
		firstLeg->commonRootPrice += cheapestFlight->fare; // на каждой итерации инкрементируем стоимость маршрута 
		//--------  new end  ------------//

		lastLeg = newLeg;
	}

	//-------- new start ------------//
	// тут отрабатываем маршруты через одну авиакомпанию
	if (carrierList.size())	{

		std::mutex firstLegLock;

		for (auto carrier : carrierList) {
	
			TransLeg  *tempLeg = new TransLeg;
			RoutePoint *routePoint = 0;
			Flight  *cheapestFlight;
			TransLeg *lastLeg = 0;

			while (route.iterator(routePoint) && routePoint->next) {

				cheapestFlight = findCheapestOneCarrierFlight(schedule, routePoint->point, 
					routePoint->next->point, *carrier);
				
				if (!cheapestFlight) {
					tempLeg = 0;
					break;
				}

				cheapestFlight->withDiscount = true; // флаг, если стоимость нужно учесть со скидкой
				TransLeg  *newLeg = new TransLeg;  
				newLeg->flight = *cheapestFlight;				
									
				if (lastLeg)
					lastLeg->next = newLeg;
				else
					tempLeg = newLeg;

				tempLeg->commonRootPrice += cheapestFlight->fare * 0.8;

				lastLeg = newLeg;
			}
			
				// если маршрут сложился
			if (tempLeg) {
					
				// и если его стоимость меньше, чем у предыдущего	
				if (firstLeg->commonRootPrice > tempLeg->commonRootPrice) {
					// удаляем предыдущий маршрут
					deleteList(firstLeg);
					delete firstLeg;
					firstLeg = tempLeg; // заменяем на новый
				}
				else {
					// если нет, удаляем новый
					deleteList(tempLeg);
					delete tempLeg;
				}
			}			
		}
	}
	//--------  new end  ------------//

	return /*new*/firstLeg ? 0 : 1;
}

void Transportation::print() const
{
	int	legNo = 0;
	//Fare	totalFare = 0;
	for( TransLeg *leg = firstLeg; leg; leg = leg->next) {
		//totalFare += leg->flight.fare;
		printf( "%d: ", legNo++ );
		leg->flight.print();
		printf( "\n" );
	}
	printf( "Total fare: %.2f\n", firstLeg->commonRootPrice ); //new скорректировано под double
}


//___

int main()
{
	// читаем маршрут
	Route	route;
	if( route.read( "route.txt" )) {
		fprintf( stderr, "cannot read route\n" );
		return 1;
	}
	route.print( "Route read: " );
	if( route.check()) {
		fprintf( stderr, "route is invalid\n" );
		return 1;
	}
	
	// читаем расписание
	Schedule	schedule;
	if( schedule.read( "schedule.txt" )) {
		fprintf( stderr, "cannot read schedule\n" );
		return 1;
	}
	printf( "\nSchedule read:\n" );
	schedule.print();
	
	// Строим перевозку
	Transportation	trans;
	if( trans.buildCheapest( route, schedule )) {
		fprintf( stderr, "cannot build transportation\n" );
		return 1;
	}
	
	printf( "\nCheapest transportation:\n" );
	trans.print();
	
	return 0;
}

