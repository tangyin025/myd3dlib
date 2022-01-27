#pragma once

#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

// CNavigationDlg dialog

class CNavigationDlg : public CDialogEx
	, public rcContext
{
	DECLARE_DYNAMIC(CNavigationDlg)

public:
	CNavigationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNavigationDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG4 };

	/// These are just sample areas to use consistent values across the samples.
	/// The use should specify these base on his needs.
	enum SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
	};
	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};
	boost::shared_ptr<rcHeightfield> m_solid;
	//boost::shared_array<char> m_triareas;
	boost::shared_ptr<rcCompactHeightfield> m_chf;
	boost::shared_ptr<rcContourSet> m_cset;
	boost::shared_ptr<rcPolyMesh> m_pmesh;
	rcConfig m_cfg;
	boost::shared_ptr<rcPolyMeshDetail> m_dmesh;
	boost::shared_ptr<dtNavMesh> m_navMesh;
	//boost::shared_ptr<dtNavMeshQuery> m_navQuery;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	/// Clears all log entries.
	virtual void doResetLog() {}

	/// Logs a message.
	///  @param[in]		category	The category of the message.
	///  @param[in]		msg			The formatted message.
	///  @param[in]		len			The length of the formatted message.
	virtual void doLog(const rcLogCategory /*category*/, const char* /*msg*/, const int /*len*/);

	/// Clears all timers. (Resets all to unused.)
	virtual void doResetTimers() {}

	/// Starts the specified performance timer.
	///  @param[in]		label	The category of timer.
	virtual void doStartTimer(const rcTimerLabel /*label*/) {}

	/// Stops the specified performance timer.
	///  @param[in]		label	The category of the timer.
	virtual void doStopTimer(const rcTimerLabel /*label*/) {}

	/// Returns the total accumulated time of the specified performance timer.
	///  @param[in]		label	The category of the timer.
	///  @return The accumulated time of the timer, or -1 if timers are disabled or the timer has never been started.
	virtual int doGetAccumulatedTime(const rcTimerLabel /*label*/) const { return -1; }

	unsigned char* buildTileMesh(const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize);

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	my::AABB m_bindingBox;
	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	int m_regionMinSize;
	int m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	int m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	BOOL m_filterLowHangingObstacles;
	BOOL m_filterLedgeSpans;
	BOOL m_filterWalkableLowHeightSpans;
	int m_partitionType;
	int m_maxTiles;
	int m_maxPolysPerTile;
	float m_tileSize;
};
