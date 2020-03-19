#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Actor.h"
#include "Vector.h"
#include "Math/Vector.h"
#include <limits>
#include <Templates/Function.h>
#include "Cell.generated.h"

struct FVector2i;

USTRUCT(BlueprintType)
struct FVector3i
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = Default)
		int32 X;

	UPROPERTY(BlueprintReadWrite, Category = Default)
		int32 Y;

	UPROPERTY(BlueprintReadWrite, Category = Default)
		int32 Z;

public:

	FVector3i() : X(0), Y(0), Z(0)
	{
	}

	static FVector3i Zero()
	{
		return { 0,0,0 };
	}

	constexpr FVector3i Abs()
	{
		return FVector3i(FMath::Abs(X), FMath::Abs(Y), FMath::Abs(Z));
	}

	constexpr FVector3i(int32 inX, int32 inY, int32 inZ)
		: X(inX), Y(inY), Z(inZ)
	{}

	constexpr explicit FVector3i(int32 value)
		: X(value), Y(value), Z(value)
	{}

	constexpr explicit FVector3i(EForceInit)
		: X(0), Y(0), Z(0)
	{}

	constexpr FVector3i(const FVector3i &other)
		: X(other.X), Y(other.Y), Z(other.Z)
	{}

	constexpr FVector3i(const FVector2i &other, int32 z);

	constexpr FVector3i(FVector3i &&other)
		: X(other.X), Y(other.Y), Z(other.Z)
	{}

	FVector3i(const FVector &other)
		: X(static_cast<int32>(other.X)), Y(static_cast<int32>(other.Y)), Z(static_cast<int32>(other.Z))
	{}

	FVector3i &operator=(const FVector3i &other)
	{
		X = other.X;
		Y = other.Y;
		Z = other.Z;

		return *this;
	}

	FVector3i &operator=(FVector3i &&other)
	{
		X = other.X;
		Y = other.Y;
		Z = other.Z;

		return *this;
	}

public:

	~FVector3i() = default;

public:

	constexpr bool operator==(const FVector3i& other) const
	{
		return X == other.X && Y == other.Y && Z == other.Z;
	}
	constexpr bool operator!=(const FVector3i& other) const
	{
		return X != other.X || Y != other.Y || Z != other.Z;
	}
	constexpr FVector3i operator*(int32 scale) const
	{
		return FVector3i(X * scale, Y * scale, Z * scale);
	}
	constexpr FVector3i operator*(const FVector3i& other) const
	{
		return FVector3i(X * other.X, Y * other.Y, Z * other.Z);
	}
	constexpr FVector3i operator/(int32 divisor) const
	{
		return FVector3i(X / divisor, Y / divisor, Z / divisor);
	}
	constexpr FVector3i operator/(const FVector3i& other) const
	{
		return FVector3i(X / other.X, Y / other.Y, Z / other.Z);
	}
	constexpr FVector3i operator+(const FVector3i& other) const
	{
		return FVector3i(X + other.X, Y + other.Y, Z + other.Z);
	}
	constexpr FVector3i operator-(const FVector3i& other) const
	{
		return FVector3i(X - other.X, Y - other.Y, Z - other.Z);
	}
	constexpr FVector3i operator-() const
	{
		return FVector3i(-X, -Y, -Z);
	}
	constexpr FVector3i operator+(int32 value) const
	{
		return FVector3i(X + value, Y + value, Z + value);
	}
	constexpr FVector3i operator-(int32 value) const
	{
		return FVector3i(X - value, Y - value, Z - value);
	}
	constexpr FVector3i operator%(int32 value) const
	{
		return FVector3i(X % value, Y % value, Z % value);
	}
	constexpr FVector3i operator&(int32 value) const
	{
		return FVector3i(X & value, Y & value, Z & value);
	}
	constexpr FVector3i operator&(const FVector3i & value) const
	{
		return FVector3i(X & value.X, Y & value.Y, Z & value.Y);
	}
	constexpr FVector3i operator|(int32 value) const
	{
		return FVector3i(X | value, Y | value, Z | value);
	}
	constexpr FVector3i operator|(const FVector3i & value) const
	{
		return FVector3i(X | value.X, Y | value.Y, Z | value.Y);
	}

	constexpr bool operator<(const FVector3i &other) const
	{
		return X != other.X ? X < other.X : (Y != other.Y ? Y < other.Y : Z < other.Z);
	}

	//   FVector3i& operator*=(int32 scale);
	//   FVector3i& operator/=(int32 divisor);
	//   FVector3i& operator+=(const FVector3i& other);
	//   FVector3i& operator-=(const FVector3i& other);
	//   FVector3i& operator=(const FVector3i& other);

	constexpr bool IsZero() const
	{
		return X == 0 && Y == 0 && Z == 0;
	}

	constexpr int32 Capacity() const
	{
		return FMath::Abs(X * Y * Z);
	}

public:

	operator FIntVector() const
	{
		return FIntVector(X, Y, Z);
	}

	friend FORCEINLINE uint32 GetTypeHash(const FVector3i& FVector3i)
	{
		return FCrc::MemCrc_DEPRECATED(&FVector3i, sizeof(FVector3i));
	}
};

inline FVector3i RoundToInt(const FVector &val)
{
	return FVector3i{ FMath::RoundToInt(val.X), FMath::RoundToInt(val.Y) , FMath::RoundToInt(val.Z) };
}

USTRUCT(BlueprintType)
struct FVector2i
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
		int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
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

	explicit constexpr FVector2i(FVector3i value)
		: X(value.X)
		, Y(value.Y)
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

constexpr FVector3i::FVector3i(const FVector2i &other, int32 z)
	: X(other.X), Y(other.Y), Z(z)
{}


using Vec3f = FVector;
using Vec3i = FVector3i;
using Vec2i = FVector2i;
using IndexType = int32;

constexpr Vec2i gSize = FVector2i(256, 256);

UENUM(BlueprintType)		
enum class EGene : uint8
{
	Trash,
	MoveForward,
	MoveBackward,
	RotateCCW,
	RotateCW,
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
	DetectOther,
	DetectEnergy,
	EGene_MAX,
};

UENUM(BlueprintType)
enum class ELense : uint8
{
	Energy,
	Age,
	Genome,
};

inline constexpr Vec2i IndexToCell(IndexType i, const Vec2i &size = gSize)
{
	return Vec2i{ static_cast<IndexType>(i / size.Y),
		static_cast<IndexType>(i % size.Y) };
}

inline constexpr IndexType CellToIndex(const Vec2i &_pos, const Vec2i &size = gSize)
{
	auto pos = _pos;
	if (pos.X >= size.X)
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
	}

	return static_cast<IndexType>(pos.X) * size.Y +
		static_cast<IndexType>(pos.Y);
}

UCLASS()
class CELLFACTORY_API UCell : public UObject
{
	GENERATED_BODY()

public:

	TArray<uint8> Genome;

	float Rotation = 0;
	FVector2D Speed = {};
	float Energy = 0;
	int32 Counter = 0;
	int32 Age = 0;

	FVector2D accumulated_delta;

	bool IsFriend(const UCell * other) const;
	bool IsOther(const UCell * other) const;
};

UCLASS()
class CELLFACTORY_API ACellActor : public AActor
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		UTexture2D * GenerateTexture(ELense lense) const;

	void Mutate(UCell *);

	UPROPERTY()
		TArray<UCell *> mArray;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		int32 LastUpdated = 0;

	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual void BeginPlay() override;

	void Repopulate();

	double max = std::numeric_limits<double>::min(), min = std::numeric_limits<double>::max();

	FRandomStream rstream;
	
	int32 time_ticks = 0;
};