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

	boost::shared_ptr<dtNavMesh> m_navMesh;

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

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
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
	afx_msg void OnChangeEdit7();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	BOOL m_tileExceeded;
};
