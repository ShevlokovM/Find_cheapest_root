#include <stdio.h>
#include <string.h>

typedef char	Carrier[3];		// ��� ������������
typedef char	FlightNo[5];	// ����� �����
typedef char	Point[4];		// ��� ������
typedef long	Fare;				// �����

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

   // ��������� ������� �� ���������� ������ ����� ��������� ��������.
   bool GetFlights(Point depPoint, Point arrPoint, Schedule& schedule) const;
};

// ������� ���������
struct TransLeg {
	TransLeg	*next;
	Flight	flight;
	
	TransLeg() : next ( 0 ) {}
};

// ���������
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

   /// ������������ ������� � ���������� ������������ (���� ����).
   static Flight	*findMainCarrierFlight( const Schedule & schedule, const char *depPoint, const char *arrPoint, Carrier mainCarrier );
};