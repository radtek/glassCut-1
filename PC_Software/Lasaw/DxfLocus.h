#pragma once

class CLocusChain 
{
	std::vector<MyNode> OrderLocus;
	std::vector<MyChain> UserOrder;
	void ReBuildUserOrder();
public:
	CLocusChain();
	virtual ~CLocusChain();
	void SetNodeSize(UINT nCont, BYTE nDir = 0);
	UINT GetNodeSize();

	BOOL SetNodeDir(UINT nIndex, BYTE nDir = 1);
	BOOL GetNodeDir(UINT nIndex, BYTE& nDir);
	BOOL GetNodeMark(UINT nIndex, UINT& nMark);
	BOOL SetEntyDir(BYTE nDir = 1);
	BOOL ReverseEntyDir();
	UINT GetEntySize();
	UINT GetChainSize(UINT nIndex);
	BOOL Combine(UINT nNodeDst, UINT nNodeTake);
	const MyNode* GetNode(UINT nChain, UINT nIndex);
};

class CDxfLocus
{
	DWORD Locus_Num;//轨迹数
public:

	CDxfLocus();
	virtual ~CDxfLocus();
	BOOL OpenDxf(CString filepath,
		double Proportion,             //CAD现实的图和绘制中的图的比例
		DWORD  Ellipse_Cut_Min_Angel,  //分割查补椭圆的最小角度
		DWORD  Spline_Cut_Step         //一段曲线要分为多少直线段
		);
	void ClosDxf();
	BOOL Analysis(BYTE nDir = 0);//0正向1逆向
	BOOL AnalysisAdv();//

	DWORD GetLocusNum();
	DWORD GetSampleNum(DWORD Locus_Num);
	CRect GetTargetRect();

	DWORD GetType(BYTE nDir,DWORD Locus_Num, DWORD  Sample_Num);

	BOOL GetLine(double *Start_x,    //直线起始点坐标的x值
		double *Start_y,    //直线起始点坐标的y值
		double *End_x,      //直线终止点坐标的x值
		double *End_y,      //直线终止点坐标的y值
		DWORD   Direction,  //轨迹的方向
		DWORD   Locus_Num,  //轨迹的序列号
		DWORD   Sample_Num);//图元的序列号
	BOOL GetArc(double * Center_x,    //圆弧圆心坐标的x值
		double * Center_y,    //圆弧圆心坐标的y值
		double * r,           //圆弧的半径
		double * Start_X,     //圆弧的起始点的X坐标
		double * Start_Y,     //圆弧的起始点的Y坐标角度
		double * End_X,       //圆弧的终止点的X坐标
		double * End_Y,       //圆弧的终止点的Y坐标
		DWORD  * MoveDirection, //运动方向
		double * Start_Angle, //圆弧的起始角度
		double * End_Angle,   //圆弧的终止角度
		DWORD    Direction,  //轨迹的方向
		DWORD    Locus_Num,   //轨迹的序列号
		DWORD    Sample_Num); //图元的序列号

	BOOL m_bIsOpen;
	CRect AreaRc;
	CString m_strFile;
	CContourTrack m_ctt;
	CLocusChain	m_LocusChain;
protected:
	void InitializeContext();
	CString ErrorInfo(UINT lErrorID);

};

