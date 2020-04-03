#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"
#include "Vector.h"
#include "Math/Vector.h"
#include <limits>
#include <Templates/Function.h>
#include <array>
#include "Cell.generated.h"

struct FVector2i
{

public:

	int32 X;
	int32 Y;

public:

	FVector2i()
		: X(0)
		, Y(0)
	{}

	constexpr FVector2i(int32 inX, int32 inY)
		: X(inX)
		, Y(inY)
	{}

	constexpr explicit FVector2i(int32 value)
		: X(value), Y(value)
	{}

	constexpr explicit FVector2i(EForceInit)
		: X(0)
		, Y(0)
	{}

	constexpr FVector2i(const FVector2i &other)
		: X(other.X)
		, Y(other.Y)
	{}

	constexpr FVector2i(const FVector2D &other)
		: X(static_cast<int>(other.X))
		, Y(static_cast<int>(other.Y))
	{}

	constexpr FVector2i(FVector2i &&other)
		: X(other.X)
		, Y(other.Y)
	{}

	FVector2i & operator = (const FVector2i &other)
	{
		X = other.X;
		Y = other.Y;

		return *this;
	}

	FVector2i & operator = (FVector2i &&other)
	{
		X = other.X;
		Y = other.Y;

		return *this;
	}

	FVector2i operator + (FVector2i &other) const
	{
		return FVector2i(X + other.X, Y + other.Y);
	}

public:

	~FVector2i() = default;

public:

	constexpr bool operator==(const FVector2i& other) const
	{
		return X == other.X && Y == other.Y;
	}
	constexpr bool operator!=(const FVector2i& other) const
	{
		return X != other.X || Y != other.Y;
	}
	constexpr FVector2i operator*(int32 scale) const
	{
		return FVector2i(X * scale, Y * scale);
	}
	constexpr FVector2i operator*(const FVector2i& other) const
	{
		return FVector2i(X * other.X, Y * other.Y);
	}
	constexpr FVector2i operator/(int32 divisor) const
	{
		return FVector2i(X / divisor, Y / divisor);
	}
	constexpr FVector2i operator/(const FVector2i& other) const
	{
		return FVector2i(X / other.X, Y / other.Y);
	}
	constexpr FVector2i operator+(const FVector2i& other) const
	{
		return FVector2i(X + other.X, Y + other.Y);
	}
	constexpr FVector2i operator-(const FVector2i& other) const
	{
		return FVector2i(X - other.X, Y - other.Y);
	}
	constexpr FVector2i operator-() const
	{
		return FVector2i(-X, -Y);
	}
	constexpr FVector2i operator+(int32 value) const
	{
		return FVector2i(X + value, Y + value);
	}
	constexpr FVector2i operator-(int32 value) const
	{
		return FVector2i(X - value, Y - value);
	}

	//   Vector3& operator*=(int32 scale);
	//   Vector3& operator/=(int32 divisor);
	//   Vector3& operator+=(const Vector3& other);
	//   Vector3& operator-=(const Vector3& other);
	//   Vector3& operator=(const Vector3& other);

	constexpr bool IsZero() const
	{
		return X == 0 && Y == 0;
	}

	constexpr int32 Capacity() const
	{
		return X * Y;
	}

public:
	operator FIntVector() const
	{
		return FIntVector(X, Y, 0);
	}

	friend FORCEINLINE uint32 GetTypeHash(const FVector2i& Vector3)
	{
		return FCrc::MemCrc_DEPRECATED(&Vector3, sizeof(FVector2i));
	}
};

using Vec2i = FVector2i;

constexpr FVector2i gSize = FVector2i(256, 256);
constexpr uint32 gGenomeSize = 64;
using GeneType = uint8;
using AgeType = uint16;

using RotationType = uint8;
constexpr RotationType gRotationsCount = 4;
constexpr std::array<Vec2i, gRotationsCount> gRotations = { Vec2i(0, 1), Vec2i(1, 0), Vec2i(0, -1), Vec2i(-1, 0) };

enum EGene : GeneType
{
	Trash,
	MoveForward,
	MoveBackward,
	Photo,
	Chemo,
	Death,
	EatForward,
	Mitose,
	GiveEnergy,
	TakeEnergy,
	Olding,
	Regen,
	Counter,
	DetectFriend,
	//DetectOther,
	DetectEnergy,
	EGene_MAX,
};

UENUM(BlueprintType)
enum class ELense : uint8
{
	Energy,
	Age,
	Genome,
	Feed,
};

inline constexpr Vec2i IndexToCell(int32 i, const Vec2i &size = gSize)
{
	return Vec2i{ static_cast<int32>(i / size.Y),
		static_cast<int32>(i % size.Y) };
}

inline constexpr int32 CellToIndex(const Vec2i &_pos, const Vec2i &size = gSize)
{
	auto pos = _pos;
	/*if (pos.X >= size.X)
	{
		pos.X = pos.X - size.X;
	}
	if (pos.Y >= size.Y)
	{
		pos.Y = pos.Y - size.Y;
	}
	if (pos.X < 0)
	{
		pos.X = pos.X + size.X;
	}
	if (pos.Y < 0)
	{
		pos.Y = pos.Y + size.Y;
	}*/

	if (pos.X >= size.X)
	{
		pos.X = pos.X - size.X;
	}
	if (pos.Y >= size.Y)
	{
		pos.X = pos.Y - size.Y;
	}
	if (pos.X < 0)
	{
		pos.X = 0;
	}
	if (pos.Y < 0)
	{
		pos.Y = 0;
	}

	return static_cast<int32>(pos.X) * size.Y +
		static_cast<int32>(pos.Y);
}

class Cell
{

public:

	std::array<uint8, gGenomeSize> Genome;

	float Energy = 0;
	uint16 Counter = 0;
	uint16 Age = 0;
	uint16 GenomeSum = 0;
	uint8 FeedType = 0;

	FVector2D accumulated_delta;

	bool IsFriend(const Cell & other) const;
	bool IsOther(const Cell & other) const;
	bool IsDead() const;
	void Kill();
	bool IsEmpty() const;
	void SetGenome(std::array<uint8, gGenomeSize> arr);
};

static std::array<Cell, gSize.Capacity()> mArray;

UCLASS()
class CELLFACTORY_API ACellActor : public AActor
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		UTexture2D * GenerateTexture(ELense lense) const;

	void Mutate(Cell & cell, bool rehash);

	float GetTime() const;
	float GetLight(int32 depth) const;
	float GetChemo(int32 depth) const;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		int32 LastUpdated = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		int32 TickUpdated = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		float TickDuration = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float SunMin = 4;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float SunMax = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float MinMax = 3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float MinMin = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Acceleration = 25;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float MutationRatio = 1;

	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual void BeginPlay() override;

	void Repopulate();

	double max = std::numeric_limits<double>::min(), min = std::numeric_limits<double>::max();

	FRandomStream rstream;

	uint64 time_ticks = 0;
};