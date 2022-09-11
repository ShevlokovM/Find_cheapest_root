#include <stdio.h>
#include <string.h>

//-------- new start ------------//
#include <set>
#include <vector>
#include <thread>
#include <mutex>

//--------  new end  ------------//

typedef char	Carrier[3];		// ��� ������������
typedef char	FlightNo[5];	// ����� �����
typedef char	Point[8];		// ��� ������
typedef /*new*/double	Fare;				// �����

// ����� ��������
struct RoutePoint {
	RoutePoint	*next;
	Point			point;
	
	RoutePoint() : next( 0 ) {}

};

// �������
class Route {
	RoutePoint	*first;
	
 public:
	Route();
	~Route();
	
	// ������ �� �����
	int read( const char *fileName );	// 0 - OK, !=0 - ������
	
	// �������� �������� ��:
	//   ������������ �������� �������
	//   �� ����� ���� ������� � ��������
	int check() const;	// 0 - OK, !=0 - ������
	
	// ��������
	RoutePoint	*iterator( RoutePoint *& iter ) const;
	
	// ������ �� stdout
	void print( const char *prefix ) const;

};

// ����
struct Flight {
	Carrier	carrier;		// ����������
	FlightNo	flightNo;	// ����� �����
	Point		depPoint;	// ����� �����������
	Point		arrPoint;	// ����� ����������
	Fare		fare;			// �����
	bool withDiscount = false; // new ���� �� �������, ����� true
	
	void	print() const;
};

//
class ScheduleItem : public Flight {
	friend class Schedule;
	
	ScheduleItem	*next;
	
 public:
	ScheduleItem() : next( 0 ) {}

};

// ����������
class Schedule {
	ScheduleItem	*firstFlight;
 public:
	Schedule();
	~Schedule();
	
	// ������ �� �����
	int read( const char *fileName );	// 0 - OK, !=0 - ������
	
	// ��������
	ScheduleItem	*iterator( ScheduleItem *& iter ) const;
	
	// ������ �� stdout
	void	print() const;
	
};

// ������� ���������
struct TransLeg {
	TransLeg	*next;
	Flight	flight;
	//-------- new start ------------//
	// ��������� ����  - ����� �������� �������� �� ��������
	// ����������� ������ � ������ ���� ������
	Fare commonRootPrice;
	//--------  new end  ------------//
	
	
	TransLeg() : next ( 0 ) 
	//-------- new start ------------//
	, commonRootPrice ( 0 ) {}
	
	//--------  new end  ------------//
};

// ���������
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
	// ������� ���� �������� ������ ����� ������������
	Flight* findCheapestOneCarrierFlight(const Schedule& schedule, const char* depPoint, const char* arrPoint, const char* carrier);
	//--------  new end  ------------//
};

//___ ���������� _________________________________

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

//___ ���������� ___________________________________________

void Flight::print() const
{
	printf( "%-2s %-4s %-3s %-3s %.2f", //new ���������� ��� double
		carrier, 
		flightNo, 
		depPoint,
		arrPoint,
		(withDiscount ? fare * 0.8 : fare) ); // new ����� ��������� �� ������� ��� ���
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

// �������� ������� �� ���������, ������ ��������� ������������, �� �������� ����� ������ ����������� �����
Flight * Transportation::findCheapestFlight( const Schedule & schedule, const char *depPoint, const char *arrPoint
	/*new start*/, std::set<Carrier*>* carrierList/*new end*/)
{
	Flight	*flightWithMinimalFare = 0;
	
	ScheduleItem	*schedItem = 0;
	while( schedule.iterator( schedItem )) {
		if(	0 != strcmp( schedItem->depPoint, depPoint ) ||
				0 != strcmp( schedItem->arrPoint, arrPoint )) continue;	
		//-------- new start ------------//
		// �������� ���� ������� ������ ���� ������������, � ������� ���� ����� �� ������ ����� �� ������
		// �� ��������� ����� ������ ����� ����� ����������� ����� ��������� � �������� � ��������
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
// ������� ���� ����� ������� �������� ���������� ������������
Flight* Transportation::findCheapestOneCarrierFlight(const Schedule& schedule, const char* depPoint, const char* arrPoint
	, const char* carrier)
{
	Flight* flightWithMinimalFare = 0;

	ScheduleItem* schedItem = 0;
	while (schedule.iterator(schedItem)) {
		if (0 != strcmp(schedItem->depPoint, depPoint) ||
			0 != strcmp(schedItem->arrPoint, arrPoint) ||
			0 != strcmp(schedItem->carrier, carrier)) continue;	// ��� ������� �� ������������ ������������	
		
		if (!flightWithMinimalFare || flightWithMinimalFare->fare > schedItem->fare) {
			flightWithMinimalFare = schedItem;
		}
	}

	return flightWithMinimalFare;
}

// ������� ������� ��� �������� ���������� ������, ����� �������
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
		// ����� ��������� ������ ��������� ������������
		// ����� ������� � ���� ������, � ������������ ������ ���� ���� �� ���� ���� �� ������� ������ �������� �� ������
		if (carrierList.size() == 0) {
			// �������, ���� ������ ������, � �� ��-���� ������ ������ �� ������ ��������,
			// �� �������� ������� ������ ������ �������� �������� � ������ ���������� ������
			cheapestFlight = findCheapestFlight(schedule, routePoint->point, routePoint->next->point, &carrierList);
		}
		else {
			cheapestFlight = findCheapestFlight(schedule, routePoint->point, routePoint->next->point);
		}
		
		if (!cheapestFlight) {
			// ������� ��������� ������, �.�. � �������� ��� �����������, � �� �� ������ �� ��������� �����
			deleteList(firstLeg);
			delete firstLeg;
			firstLeg = 0;
			break; // ���� ������ ��������� ����, ����� ����� ��� ���������� �������� � ����� �������������
		}
		//--------  new end  ------------//

		TransLeg	*newLeg = new TransLeg;
		newLeg->flight = *cheapestFlight;
		

		if( lastLeg ) 
			lastLeg->next = newLeg;
		else
			firstLeg = newLeg;

		//-------- new start ------------//
		firstLeg->commonRootPrice += cheapestFlight->fare; // �� ������ �������� �������������� ��������� �������� 
		//--------  new end  ------------//

		lastLeg = newLeg;
	}

	//-------- new start ------------//
	// ��� ������������ �������� ����� ���� ������������
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

				cheapestFlight->withDiscount = true; // ����, ���� ��������� ����� ������ �� �������
				TransLeg  *newLeg = new TransLeg;  
				newLeg->flight = *cheapestFlight;				
									
				if (lastLeg)
					lastLeg->next = newLeg;
				else
					tempLeg = newLeg;

				tempLeg->commonRootPrice += cheapestFlight->fare * 0.8;

				lastLeg = newLeg;
			}
			
				// ���� ������� ��������
			if (tempLeg) {
					
				// � ���� ��� ��������� ������, ��� � �����������	
				if (firstLeg->commonRootPrice > tempLeg->commonRootPrice) {
					// ������� ���������� �������
					deleteList(firstLeg);
					delete firstLeg;
					firstLeg = tempLeg; // �������� �� �����
				}
				else {
					// ���� ���, ������� �����
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
	printf( "Total fare: %.2f\n", firstLeg->commonRootPrice ); //new ��������������� ��� double
}


//___

int main()
{
	// ������ �������
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
	
	// ������ ����������
	Schedule	schedule;
	if( schedule.read( "schedule.txt" )) {
		fprintf( stderr, "cannot read schedule\n" );
		return 1;
	}
	printf( "\nSchedule read:\n" );
	schedule.print();
	
	// ������ ���������
	Transportation	trans;
	if( trans.buildCheapest( route, schedule )) {
		fprintf( stderr, "cannot build transportation\n" );
		return 1;
	}
	
	printf( "\nCheapest transportation:\n" );
	trans.print();
	
	return 0;
}

