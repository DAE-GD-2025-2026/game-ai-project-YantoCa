#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	// TODO create the cells
	CellOrigin = FVector2D(-SpaceWidth /2.f, -SpaceHeight /2.f);
	for (int row = 0; row < NrOfRows; ++row)
	{
		for (int col = 0; col < NrOfCols; ++col)
		{
			float left = CellOrigin.X + (col * CellWidth);
			float bottom = CellOrigin.Y + (row * CellHeight);

			Cells.emplace_back(left, bottom, CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	// TODO Add the agent to the correct cell
	int index = PositionToIndex(Agent.GetPosition());
		Cells[index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	//TODO Check if the agent needs to be moved to another cell.
	//TODO Use the calculated index for oldPos and currentPos for this
	int oldIndex = PositionToIndex(OldPos);
	int newIndex = PositionToIndex(Agent.GetPosition());

	if (oldIndex != newIndex){ 
		Cells[oldIndex].Agents.remove(&Agent);
		Cells[newIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	// TODO Register the neighbors for the provided agent
	// TODO Only check the cells that are within the radius of the neighborhood

	NrOfNeighbors = 0;
	
	const FVector2D agentPos = Agent.GetPosition();
	const float queryRadSq = QueryRadius * QueryRadius;

	FRect queryBox;
	queryBox.Min = agentPos - FVector2D(QueryRadius, QueryRadius);
	queryBox.Max = agentPos + FVector2D(QueryRadius, QueryRadius);

	for (Cell& cell : Cells)
	{
		if (!DoRectsOverlap(cell.BoundingBox, queryBox))
			continue;

		for (ASteeringAgent* pOther : cell.Agents)
		{
			if (pOther == &Agent)
				continue;

			if (FVector2D::DistSquared(agentPos, pOther->GetPosition()) < queryRadSq)
			{
				if (NrOfNeighbors < Neighbors.Num())
				{
					Neighbors[NrOfNeighbors++] = pOther;
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	// TODO Render the cells with the number of agents inside of it
	for (const Cell& cell : Cells)
	{
		auto rect = cell.GetRectPoints();

		DrawDebugLine(pWorld, FVector(rect[0],0), FVector(rect[1],0), FColor::Red);
		DrawDebugLine(pWorld, FVector(rect[1],0), FVector(rect[2],0), FColor::Red);
		DrawDebugLine(pWorld, FVector(rect[2],0), FVector(rect[3],0), FColor::Red);
		DrawDebugLine(pWorld, FVector(rect[3],0), FVector(rect[0],0), FColor::Red);
			
		FVector center((cell.BoundingBox.Min.X + cell.BoundingBox.Max.X) * 0.5f, (cell.BoundingBox.Min.Y + cell.BoundingBox.Max.Y) * 0.5f, 0.5f);
		
		DrawDebugString(pWorld, center, FString::FromInt(static_cast<int>(cell.Agents.size())), nullptr,FColor::White, 0.f,false,1.8f);
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	// TODO Calculate the index of the cell based on the position
	int col = int((Pos.X - CellOrigin.X) / CellWidth);
	int row = int((Pos.Y - CellOrigin.Y) / CellHeight);

	col = FMath::Clamp(col, 0, NrOfCols - 1);
	row = FMath::Clamp(row, 0, NrOfRows - 1);

	return (row * NrOfCols) + col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}