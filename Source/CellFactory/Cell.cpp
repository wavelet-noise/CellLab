#include "Cell.h"
#include "Math/UnrealMathUtility.h"
#include <Serialization/BulkData.h>
#include <TextureResource.h>
#include <Engine/Engine.h>
#include <type_traits>


UTexture2D * ACellActor::GenerateTexture(ELense lense) const
{
	UTexture2D * generated;
	if (lense == ELense::Age)
	{
		generated = UTexture2D::CreateTransient(gSize.Y, 1);
	}
	else
	{
		generated = UTexture2D::CreateTransient(gSize.X, gSize.Y);
	}
	//generated->Filter = TextureFilter::TF_Nearest;
	generated->UpdateResource();

	if (mArray.Num() > 0)
	{
		FTexture2DMipMap& GeneratedMip = generated->PlatformData->Mips[0];
		void * GeneratedData = GeneratedMip.BulkData.Lock(LOCK_READ_WRITE);

		const int32 buffer_count = GeneratedMip.BulkData.GetElementCount() * GeneratedMip.BulkData.GetElementSize();

		TArray<uint8> pData;
		pData.SetNum(buffer_count);

		for (int j = 0; j < buffer_count; j += 4)
		{
			uint8 v = 0;
			size_t cache = 0;
			switch (lense)
			{
			case ELense::Energy:
				v = mArray[j / 4] != nullptr ? FMath::Clamp(int32((mArray[j / 4]->Energy / 100.f) * 255), 0, 255) : 0;
				if (v == 0)
				{
					pData[j] = 0; //B
					pData[j + 1] = 0; //G
					pData[j + 2] = 0; //R
					continue;
				}
				pData[j] = 0;
				pData[j + 1] = (mArray[j / 4]->Genome.Num() == 0) * 255;
				pData[j + 2] = v;
				break;
			case ELense::Age:
			{
				auto depth = FMath::Abs((IndexToCell(j / 4).Y - (gSize.Y / 2)) / float((gSize.Y / 2)));
				auto time = time_ticks / 1000.f;
				auto photot = (FMath::Abs(FMath::Cos(time) + FMath::Sin(time * 4) / 2.f) * 1.5f * (1 - depth)) + 0.3f;
				auto chemenergy = depth * 1.f * FMath::Abs(FMath::Cos(1.5f + time));
				chemenergy += FMath::Clamp((10000.f - LastUpdated) / 10000.f, 0.f, 1.f);
				chemenergy = FMath::Clamp(chemenergy, 0.f, 1.f);
				photot = FMath::Clamp(photot, 0.f, 1.f);
				pData[j] = chemenergy * 255;// *0.5f * 3.f + 1.f * 51; //B
				pData[j + 1] = photot * 255; //G
				pData[j + 2] = photot * 255; //R
			}
			break;
			case ELense::Genome:
				if (mArray[j / 4] == nullptr || mArray[j / 4]->Genome.Num() == 0)
				{
					pData[j] = 0; //B
					pData[j + 1] = 0; //G
					pData[j + 2] = 0; //R
					continue;
				}
				for (auto gg : mArray[j / 4]->Genome)
				{
					cache += gg;
				}
				std::hash<uint8> hasher;
				cache = hasher(cache);
				pData[j] = cache % 255;
				pData[j + 1] = (cache / 255) % 255;
				pData[j + 2] = (cache / 255 / 255) % 255;
				break;
			default:
				pData[j] = 0; //B
				pData[j + 1] = 0; //G
				pData[j + 2] = 0; //R
				break;
			}
		}

		FMemory::Memcpy(GeneratedData, &pData[0], pData.Num() * sizeof(uint8));

		GeneratedMip.BulkData.Unlock();
		generated->UpdateResource();
	}

	return generated;
}

void ACellActor::Mutate(UCell * ncell)
{
	auto sw = rstream.RandRange(0, 2);
	switch (sw)
	{
	case 0:
		ncell->Genome.Add(rstream.RandRange(0, std::numeric_limits<uint8>::max()));
		break;
	case 1:
		ncell->Genome[rand() % ncell->Genome.Num()] = rstream.RandRange(0, std::numeric_limits<uint8>::max());
		break;
	case 2:
		ncell->Genome.Pop();
		break;
	default:
		break;
	}
}

void ACellActor::GetChemo(int32 depth)
{

}

void ACellActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//auto tick1 = FPlatformTime::Seconds();

	for (int32 iter = 0; iter < 25; ++iter)
	{
		++time_ticks;
		auto time = time_ticks / 1000.f;

		mArray[0] = nullptr;
		auto updated = 0;

		for (int32 j = 0; j < gSize.Y; ++j)
		{
			auto depth = FMath::Abs((j - (gSize.Y / 2)) / float((gSize.Y / 2)));
			auto photoenergy = (FMath::Abs(FMath::Cos(time) + FMath::Sin(time * 4) / 2.f) * 1.5f * (1 - depth));
			auto chemenergy = depth * 1.f * FMath::Abs(FMath::Cos(1.5f + time));
			chemenergy += FMath::Clamp((10000.f - LastUpdated) / 10000.f, 0.f, 1.f);
			//photoenergy *= (30000.f - LastUpdated) / 30000.f;
			//chemenergy *= (30000.f - LastUpdated) / 30000.f;
			for (int32 i = 0; i < gSize.X; ++i)
			{
				auto self_index = CellToIndex({ i, j });

				if (mArray[self_index] != nullptr)
				{
					auto cell = mArray[self_index];

					cell->accumulated_delta += cell->Speed;
					cell->Speed *= 0.99;

					if (cell->Genome.Num() > 0)
					{
						++updated;

						bool jumped = false;
					single_jump:
						const auto command1 = cell->Genome[cell->Counter % cell->Genome.Num()];
						if (jumped && (command1 == uint8(EGene::Counter) || command1 == uint8(EGene::DetectEnergy) || command1 == uint8(EGene::DetectFriend) || command1 == uint8(EGene::DetectOther)))
						{
							goto double_jump;
						}

						const auto i_param1 = cell->Genome[(cell->Counter + 1) % cell->Genome.Num()];
						const auto param1 = i_param1 / float(std::numeric_limits<uint8>::max());
						const auto i_param2 = cell->Genome[(cell->Counter + 2) % cell->Genome.Num()];
						const auto param2 = i_param2 / float(std::numeric_limits<uint8>::max());

						switch (command1)
						{
						case uint8(EGene::MoveForward):
						{
							auto nvec = FVector2D(1, 0).GetRotated(cell->Rotation) * 0.1;
							cell->Speed += nvec;
							cell->Energy -= nvec.Size();
							cell->Counter += 1;
						}
						break;

						case uint8(EGene::Olding):
						{
							cell->Age += 10 * param1;
							cell->Counter += 2;
						}
						break;

						case uint8(EGene::Photo):
						{
							cell->Energy += photoenergy;
							cell->Counter += 1;
						}
						break;

						case uint8(EGene::Chemo):
						{
							cell->Energy += chemenergy;
							cell->Counter += 1;
						}
						break;

						case uint8(EGene::Mitose):
						{
							auto rot = FVector2D(1, 0).GetRotated(cell->Rotation + param1 * 360);
							auto irot = Vec2i(FMath::RoundFromZero(rot.X), FMath::RoundFromZero(rot.Y));
							auto n_index = CellToIndex({ i - irot.X, j - irot.Y });

							if (mArray[n_index] == nullptr && cell->Energy > 1)
							{
								auto ncell = NewObject<UCell>(this);
								ncell->Genome = cell->Genome;

								if (rstream.RandRange(0, cell->Age) > 100 * cell->Genome.Num())
								{
									Mutate(ncell);
									Mutate(cell);
								}
								ncell->Speed = ncell->Speed;
								ncell->Rotation = cell->Rotation + param1 * 360;
								ncell->Energy = cell->Energy * param2 * 0.8;
								mArray[n_index] = ncell;
								cell->Energy = cell->Energy * (1 - param2) * 0.8;
								cell->Age = 0;
							}

							cell->Counter += 3;
						}
						break;

						case uint8(EGene::RotateCW):
						{
							cell->Rotation += param1 * 360;
							cell->Energy -= param1 * 0.1;

							cell->Counter += 2;
						}
						break;

						case uint8(EGene::RotateCCW):
						{
							cell->Rotation -= param1 * 360;
							cell->Energy -= param1 * 0.1;

							cell->Counter += 2;
						}
						break;

						case uint8(EGene::GiveEnergy):
						{
							auto rot = FVector2D(1, 0).GetRotated(cell->Rotation + param1 * 360);
							auto irot = Vec2i(FMath::RoundFromZero(rot.X), FMath::RoundFromZero(rot.Y));
							auto n_index = CellToIndex({ i - irot.X, j - irot.Y });
							if (mArray[n_index] != nullptr && n_index != self_index)
							{
								auto ncell = mArray[n_index];

								ncell->Energy += cell->Energy * param2 * 0.75;
								cell->Energy -= cell->Energy * param2;
							}

							cell->Counter += 3;
						}
						break;

						case uint8(EGene::Regen):
						{
							cell->Age *= param1;
							cell->Energy *= param1;

							cell->Counter += 2;
						}
						break;

						case uint8(EGene::TakeEnergy):
						{
							auto rot = FVector2D(1, 0).GetRotated(cell->Rotation + param1 * 360);
							auto irot = Vec2i(FMath::RoundFromZero(rot.X), FMath::RoundFromZero(rot.Y));
							auto n_index = CellToIndex({ i - irot.X, j - irot.Y });
							if (mArray[n_index] != nullptr && n_index != self_index)
							{
								auto ncell = mArray[n_index];

								cell->Energy += ncell->Energy * param2 * 0.75;
								ncell->Energy -= ncell->Energy * param2;
							}

							cell->Counter += 3;
						}
						break;

						case uint8(EGene::DetectFriend):
						{
							auto rot = FVector2D(1, 0).GetRotated(cell->Rotation + param1 * 360);
							auto irot = Vec2i(FMath::RoundFromZero(rot.X), FMath::RoundFromZero(rot.Y));
							auto n_index = CellToIndex({ i - irot.X, j - irot.Y });
							if (mArray[n_index] != nullptr && n_index != self_index)
							{
								auto ncell = mArray[n_index];

								if (ncell->IsFriend(cell))
								{
									cell->Counter = i_param2;
									jumped = true;
									goto single_jump;
								}
							}

							cell->Counter += 3;
							jumped = true;
							goto single_jump;
						}
						break;

						case uint8(EGene::Counter):
						{
							cell->Counter = i_param1;
							jumped = true;
							goto single_jump;
						}
						break;

						case uint8(EGene::DetectOther):
						{
							auto rot = FVector2D(1, 0).GetRotated(cell->Rotation + param1 * 360);
							auto irot = Vec2i(FMath::RoundFromZero(rot.X), FMath::RoundFromZero(rot.Y));
							auto n_index = CellToIndex({ i - irot.X, j - irot.Y });
							if (mArray[n_index] != nullptr && n_index != self_index)
							{
								auto ncell = mArray[n_index];

								if (ncell->IsOther(cell))
								{
									cell->Counter = i_param2;
									jumped = true;
									goto single_jump;
								}
							}

							cell->Counter += 3;
							jumped = true;
							goto single_jump;
						}
						break;

						case uint8(EGene::DetectEnergy):
						{
							if (cell->Energy >= param1 * 100)
							{
								cell->Counter = i_param2;
								jumped = true;
								goto single_jump;
							}

							cell->Counter += 3;
							jumped = true;
							goto single_jump;
						}
						break;
						}

					double_jump:

						if (rstream.RandRange(0, cell->Age) > 200 * cell->Genome.Num())
						{
							Mutate(cell);
						}

						if (cell->accumulated_delta.X > 1)
						{
							auto n_index = CellToIndex({ i + 1, j });
							if (mArray[n_index] == nullptr)
							{
								cell->accumulated_delta.X -= 1;
								mArray[self_index] = nullptr;
								mArray[n_index] = cell;
							}
							else
							{
								//mArray[n_index]->Speed += cell->Speed * 0.8f;
								cell->Speed = {};
								cell->accumulated_delta = {};
							}
						}
						else if (cell->accumulated_delta.X < -1)
						{
							auto n_index = CellToIndex({ i - 1, j });
							if (mArray[n_index] == nullptr)
							{
								cell->accumulated_delta.X += 1;
								mArray[self_index] = nullptr;
								mArray[n_index] = cell;
							}
							else
							{
								//mArray[n_index]->Speed += cell->Speed * 0.8f;
								cell->Speed = {};
								cell->accumulated_delta = {};
							}
						}
						else if (cell->accumulated_delta.Y < -1)
						{
							auto n_index = CellToIndex({ i, j - 1 });
							if (mArray[n_index] == nullptr)
							{
								cell->accumulated_delta.Y += 1;
								mArray[self_index] = nullptr;
								mArray[n_index] = cell;
							}
							else
							{
								//mArray[n_index]->Speed += cell->Speed * 0.8f;
								cell->Speed = {};
								cell->accumulated_delta = {};
							}
						}
						else if (cell->accumulated_delta.Y > 1)
						{
							auto n_index = CellToIndex({ i, j + 1 });
							if (mArray[n_index] == nullptr)
							{
								cell->accumulated_delta.Y -= 1;
								mArray[self_index] = nullptr;
								mArray[n_index] = cell;
							}
							else
							{
								//mArray[n_index]->Speed += cell->Speed * 0.8f;
								cell->Speed = {};
								cell->accumulated_delta = {};
							}
						}

						cell->Energy -= 1 * (cell->Genome.Num() / 100.f);
						cell->Age += 1;

						if (cell->Energy > 100)
						{
							cell->Energy *= .9995;
						}

						//if (cell->Age > 500)
						//{
						//	if (rstream.RandRange(0, cell->Energy) == 1)
						//	{
						//		//if (rstream.RandRange(0, 1) == 1)
						//		//{
						//		//	cell->Genome.Pop();
						//		//	//cell->Genome[rand() % cell->Genome.Num()] = rstream.RandRange(0, std::numeric_limits<uint8>::max());
						//		//	cell->Age = 0;
						//		//}
						//		//else
						//		//{
						//		cell->Genome.Empty();
						//		cell->Age = 0;
						//		//}
						//	}
						//	else
						//	{

						//	}
						//}
					}
					else
					{
						cell->Energy -= 0.05f;
					}

					if (cell->Energy < 1)
					{
						if (cell->Genome.Num() > 0)
						{
							cell->Genome.Empty();
							cell->Age = 0;
							cell->Energy = 10;
						}
						else
						{
							mArray[self_index] = nullptr;
							cell->MarkPendingKill();
							continue;
						}
					}
				}
			}
		}

		LastUpdated = updated;
		if (updated == 0)
		{
			Repopulate();
		}
	}

	//auto tick2 = FPlatformTime::Seconds();
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, FString::Printf(TEXT("%f"), tick2 - tick1));

	//max = FMath::Max(max, tick2 - tick1);
	//min = FMath::Min(min, tick2 - tick1);

	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("%f"), max));
	//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, FString::Printf(TEXT("%f"), min));
}

void ACellActor::BeginPlay()
{
	Super::BeginPlay();

	rstream.GenerateNewSeed();

	mArray.SetNum(gSize.Capacity());

	Repopulate();
}

void ACellActor::Repopulate()
{
	auto num = mArray.Num();
	mArray.Empty();
	mArray.SetNumZeroed(num);

	time_ticks = 0;

	TArray<uint8> ggg;
	/*0*/ggg.Add(uint8(EGene::Photo));
	/*1*/ggg.Add(uint8(EGene::DetectEnergy));
	/*2*/ggg.Add(100);
	/*3*/ggg.Add(6);
	/*4*/ggg.Add(uint8(EGene::Counter));
	/*5*/ggg.Add(0);
	/*6*/ggg.Add(uint8(EGene::Mitose));
	/*7*/ggg.Add(0);
	/*8*/ggg.Add(128);
	/*0*/ggg.Add(uint8(EGene::Chemo));

	for (int32 i = 0; i < 10000; ++i)
	{
		auto ncell = NewObject<UCell>(this);
		ncell->Speed = { rstream.GetFraction(),rstream.GetFraction() };
		ncell->Rotation = rstream.GetFraction() * 360;
		ncell->Energy = rstream.GetFraction() * 100;

		ncell->Genome.Add(uint8(EGene::Photo));
		auto rval = rstream.RandRange(0, 10);
		for (int g = 0; g < rval + 1; ++g)
		{
			ncell->Genome.Add(rstream.RandRange(0, uint8(EGene::EGene_MAX) - 1));
			ncell->Genome.Add(rstream.RandRange(0, std::numeric_limits<uint8>::max()));
			ncell->Genome.Add(rstream.RandRange(0, std::numeric_limits<uint8>::max()));
		}
		//ncell->Genome.Append(ggg);

		mArray[rstream.GetFraction() * mArray.Num()] = ncell;
	}
}

bool UCell::IsFriend(const UCell * other) const
{
	return other->Genome.Num() == Genome.Num();
}

bool UCell::IsOther(const UCell * other) const
{
	return true;
}
