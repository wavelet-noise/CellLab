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

	//if (mArray.Num() > 0)
	{
		FTexture2DMipMap& GeneratedMip = generated->PlatformData->Mips[0];
		void * GeneratedData = GeneratedMip.BulkData.Lock(LOCK_READ_WRITE);

		const int32 buffer_count = GeneratedMip.BulkData.GetElementCount() * GeneratedMip.BulkData.GetElementSize();

		TArray<uint8> pData;
		pData.SetNum(buffer_count);

		static std::array<FColor, 6> colors = {FColor::Silver, FColor::Green, FColor::Blue, FColor::Purple, FColor::Red, FColor::Yellow};

		for (int j = 0; j < buffer_count; j += 4)
		{
			uint8 v = 0;
			size_t cache = 0;
			switch (lense)
			{
			case ELense::Feed:
				pData[j] = colors[mArray[j / 4].FeedType].B;
				pData[j + 1] = colors[mArray[j / 4].FeedType].G;
				pData[j + 2] = colors[mArray[j / 4].FeedType].R;
				break;
			case ELense::Energy:
				pData[j] = (mArray[j / 4].IsDead() && mArray[j / 4].Energy > 0) * 255;
				pData[j + 1] = mArray[j / 4].IsDead() * 255;
				pData[j + 2] = (FMath::Clamp(mArray[j / 4].Energy, 0.f, 100.f) / 100.f) * 255;
				break;
			case ELense::Age:
			{
				pData[j] = GetChemo(IndexToCell(j / 4).Y) * 127;// *0.5f * 3.f + 1.f * 51; //B
				pData[j + 1] = GetLight(IndexToCell(j / 4).Y) * 127; //G
				pData[j + 2] = GetLight(IndexToCell(j / 4).Y) * 127; //R
			}
			break;
			case ELense::Genome:
				if (mArray[j / 4].IsDead())
				{
					pData[j] = 0; //B
					pData[j + 1] = 0; //G
					pData[j + 2] = 0; //R
					continue;
				}
				std::hash<uint8> hasher;
				cache = hasher(mArray[j / 4].GenomeSum);
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

void ACellActor::Mutate(Cell & cell, bool rehash)
{
	cell.Genome[rstream.RandHelper(gGenomeSize)] = rstream.RandHelper(std::numeric_limits<GeneType>::max());
	if (rehash)
	{
		cell.SetGenome(cell.Genome);
	}
}

float ACellActor::GetTime() const
{
	return time_ticks / 1000.f;
}

float ACellActor::GetLight(int32 depth) const
{
	return (FMath::Abs((FMath::Cos(GetTime()) + FMath::Sin(GetTime() * 4) + 2) / 4.f) * SunMax * (1 - (depth / float(gSize.Y)))) + SunMin;
}

float ACellActor::GetChemo(int32 depth) const
{
	auto chemenergy = (depth / float(gSize.Y)) * MinMax + MinMin;
	return chemenergy;
}

void ACellActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	auto tick1 = FPlatformTime::Seconds();
	TickUpdated = 0;

	for (int32 iter = 0; iter < Acceleration; ++iter)
	{
		++time_ticks;

		auto updated = 0;

		for (int32 j = 0; j < gSize.Y; ++j)
		{
			auto photoenergy = GetLight(j);
			auto chemenergy = GetChemo(j);
			for (int32 i = 0; i < gSize.X; ++i)
			{
				auto self_index = CellToIndex({ i, j });

				//if (!mArray[self_index].IsDead())
				{
					auto & cell = mArray[self_index];

					if (!cell.IsDead())
					{
						++updated;
						++TickUpdated;

						//bool jumped = false;
					//single_jump:
						const auto command1 = cell.Genome[cell.Counter % gGenomeSize];
						//if (jumped && (command1 == EGene::Counter || command1 == EGene::DetectEnergy || command1 == EGene::DetectFriend || command1 == EGene::DetectOther))
						//{
						//	goto double_jump;
						//}

						const auto i_param1 = cell.Genome[(cell.Counter + 1) % gGenomeSize];
						const auto param1 = i_param1 / float(std::numeric_limits<GeneType>::max());
						const auto i_param2 = cell.Genome[(cell.Counter + 2) % gGenomeSize];
						const auto param2 = i_param2 / float(std::numeric_limits<GeneType>::max());

						auto oldc = cell.Counter;
						
						switch (command1)
						{
						case EGene::MoveForward:
						{
							cell.Energy -= 0.5;
							cell.Counter += 2;

							auto new_index = CellToIndex(Vec2i(i, j) + gRotations[i_param1 % gRotationsCount]);
							std::swap(mArray[self_index], mArray[new_index]);

							self_index = new_index;
							cell = mArray[new_index];
						}
						break;

						case EGene::Olding:
						{
							cell.Age += 10 * param1;
							cell.Counter += 2;
						}
						break;

						case EGene::Photo:
						{
							cell.Energy += photoenergy;
							cell.Counter += 1;
							cell.FeedType = 1;
						}
						break;

						case EGene::Chemo:
						{
							cell.Energy += chemenergy;
							cell.Counter += 1;
							cell.FeedType = 2;
						}
						break;

						case EGene::Mitose:
						{
							if (cell.Age > 10)
							{
								auto npos = Vec2i(i, j) + gRotations[i_param1 % gRotationsCount];
								auto n_index = CellToIndex(npos);
								if (mArray[n_index].IsEmpty())
								{
									if (cell.Energy > 1)
									{
										auto & ncell = mArray[n_index];
										ncell.SetGenome(cell.Genome);

										if (rstream.RandRange(0, 10 * MutationRatio) == 1)
										{
											Mutate(ncell, true);
										}
										if (rstream.RandRange(0, 10 * MutationRatio) == 1)
										{
											Mutate(cell, true);
										}
										ncell.Energy = cell.Energy * 0.5;
										mArray[n_index] = ncell;
										cell.Energy = cell.Energy * 0.5;
										cell.Age = 0;
										ncell.Age = 0;
									}
								}
							}

							cell.Counter += 3;
						}
						break;

						case EGene::GiveEnergy:
						{
							auto npos = Vec2i(i, j) + gRotations[i_param1 % gRotationsCount];
							auto n_index = CellToIndex(npos);
							if (!mArray[n_index].IsEmpty() && n_index != self_index)
							{
								auto ncell = mArray[n_index];

								ncell.Energy += cell.Energy * 0.5f;
								cell.Energy -= cell.Energy * 0.5f;
								cell.FeedType = 3;
							}

							cell.Counter += 3;
						}
						break;

						case EGene::Regen:
						{
							cell.Age *= param1;
							cell.Energy *= param1;

							cell.Counter += 2;
						}
						break;

						case EGene::TakeEnergy:
						{
							auto npos = Vec2i(i, j) + gRotations[i_param1 % gRotationsCount];
							auto n_index = CellToIndex(npos);
							if (!mArray[n_index].IsEmpty() && n_index != self_index)
							{
								auto ncell = mArray[n_index];

								if (!ncell.IsDead())
								{
									if (cell.IsFriend(ncell))
									{
										cell.Energy += ncell.Energy * 0.5f * 0.75f;
										cell.FeedType = 3;
									}
									else
									{
										cell.Energy += ncell.Energy * 0.5f;
										cell.FeedType = 4;
									}
								}
								else
								{
									cell.Energy += ncell.Energy * 0.5f;
									cell.FeedType = 5;
								}
								ncell.Energy -= ncell.Energy * 0.5f;
							}

							cell.Counter += 3;
						}
						break;

						case EGene::DetectFriend:
						{
							auto npos = Vec2i(i, j) + gRotations[i_param1 % gRotationsCount];
							auto n_index = CellToIndex(npos);
							if (!mArray[n_index].IsEmpty() && n_index != self_index)
							{
								auto ncell = mArray[n_index];

								if (ncell.IsFriend(cell))
								{
									cell.Counter = i_param2;
									//jumped = true;
									//goto single_jump;
								}
							}

							cell.Counter += 3;
							//jumped = true;
							//goto single_jump;
						}
						break;

						case EGene::Counter:
						{
							cell.Counter = i_param1;
							//jumped = true;
							//goto single_jump;
						}
						break;

						//case EGene::DetectOther:
						//{
						//	auto npos = Vec2i(i, j) + gRotations[cell.Rotation % 8];
						//	auto n_index = CellToIndex(npos);
						//	if (!mArray[n_index].IsEmpty() && n_index != self_index)
						//	{
						//		auto ncell = mArray[n_index];

						//		if (ncell.IsOther(cell))
						//		{
						//			cell.Counter = i_param2;
						//			//jumped = true;
						//			//goto single_jump;
						//		}
						//	}

						//	cell.Counter += 3;
						//	//jumped = true;
						//	//goto single_jump;
						//}
						//break;

						case EGene::Death:
						{
							cell.Genome[0] = EGene::Death;
						}

						case EGene::DetectEnergy:
						{
							if (cell.Energy >= param1 * 100)
							{
								cell.Counter = i_param2;
								//jumped = true;
								//goto single_jump;
							}

							cell.Counter += 3;
							//jumped = true;
							//goto single_jump;
						}
						break;
						}

					//double_jump:

						if (oldc == cell.Counter)
						{
							++cell.Counter;
						}

						if (rstream.RandRange(0, cell.Age) > 1000)
						{
							Mutate(cell, true);
							cell.Age = 0;
						}

						cell.Age += 1;

						//if (cell.Energy > 100 && rstream.RandHelper(100) == 1)
						//{
						//	//cell.Energy = 110;
						//	cell.Genome[0] = EGene::Death;
						//	//Mutate(cell, false);
						//}

						cell.Energy -= 0.5f;
					}
					else
					{
						cell.Energy *= .99f;
						cell.Energy -= 0.01f;
					}

					if (cell.Energy < 1)
					{
						cell.Kill();
						cell.Energy = 0;
					}
				}
			}
		}

		LastUpdated = updated;
		if (updated < 20)
		{
			Repopulate();
		}
	}

	auto tick2 = FPlatformTime::Seconds();
	TickDuration = tick2 - tick1;

	if (LastUpdated < 300)
	{
		Repopulate();
	}
}

void ACellActor::BeginPlay()
{
	Super::BeginPlay();

	rstream.GenerateNewSeed();

	Repopulate();
}

void ACellActor::Repopulate()
{
	time_ticks = 0;

	for (int i = 0; i < gSize.Capacity(); ++i)
	{
		mArray[i].Genome[0] = EGene::Death;
		mArray[i].Age = 0;
		mArray[i].Energy = -1;
	}

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
	/*4*/ggg.Add(uint8(EGene::Counter));
	/*5*/ggg.Add(0);

	for (int32 i = 0; i < 10000; ++i)
	{
		Cell ncell;
		ncell.Energy = rstream.GetFraction() * 100;

		for (int g = 0; g <gGenomeSize; ++g)
		{
			ncell.Genome[g] = rstream.RandHelper(std::numeric_limits<GeneType>::max());
		}
		ncell.Genome[0] = EGene::Photo;
		ncell.SetGenome(ncell.Genome);

		/*for (int g = 0; g < ggg.Num(); ++g)
		{
			ncell.Genome[g] = ggg[g];
		}*/

		mArray[rstream.RandHelper(gSize.Capacity())] = std::move(ncell);
	}
}

bool Cell::IsFriend(const Cell & other) const
{
	return GenomeSum == other.GenomeSum;
}

bool Cell::IsOther(const Cell & other) const
{
	return true;
}

bool Cell::IsDead() const
{
	return Genome[0] == EGene::Death;
}

void Cell::Kill()
{
	Genome[0] = EGene::Death;
	accumulated_delta = {};
	GenomeSum = 0;
}

bool Cell::IsEmpty() const
{
	return IsDead() && Energy <= 0;
}

void Cell::SetGenome(std::array<uint8, gGenomeSize> arr)
{
	Genome = arr;

	GenomeSum = 0;
	for (auto gg : arr)
	{
		GenomeSum += gg;
	}
}
