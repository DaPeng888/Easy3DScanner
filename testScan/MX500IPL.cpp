/************************************************************************/
/* ʵ�ֶ�MX500����ĳ�ʼ�����ɼ�ͼ��ķ�װ									*/
/* ���IplImageͼ��														*/
/* Code RFL																*/
/************************************************************************/

#include "MX500Ipl.h"

//˽�б���
static int m_nCameraID;						//�豸ID
static tDSCameraDevInfo m_dsCam[10];		//�豸��Ϣ�ṹ��
/*static */tDSImageSize m_dsImg;				//����ߴ���Ϣ
IplImage *_retireImage = NULL;


/*************************************************************************************/
/* ���ܣ�����MX500																	 */
/* ����ֵ���ɹ�����0��ʧ�ܷ���-1														 */
/*************************************************************************************/
int ConfigMX500(void)
{
	//ͼ����Ϣ
	BOOL bROI;			//�Ƿ���ROI

	//�������
	int nTotal = 0;
	emDSCameraStatus status = CameraGetDevList(m_dsCam, &nTotal);
	if (status == STATUS_OK && nTotal > 0)
	{
		status = STATUS_NOT_SUPPORTED;					//��״̬Ϊ��Ч
		printf("[MX500] camera:%s\n", m_dsCam[0].acFriendlyName);
	}
	else
	{
		printf("[MX500] found camera error\n");
		return -1;
	}

	// ��ʼ�����(��Ԥ��ͼ��)�����Ų�׽ģʽ	
	status = CameraInitEx(m_dsCam[0].acFriendlyName, &m_nCameraID);			
	if (status == STATUS_OK)
	{
		//�������Դ
		CameraPowerUp(m_nCameraID);

		//��������ֱ���,ԭʼ�ֱ���/2^n
		//CameraSetImageSizeSel(m_nCameraID, 1, FALSE);  // Ԥ���ֱ���

		// ��ȡͼ��ߴ�
		CameraGetImageSize(m_nCameraID, &bROI, &(m_dsImg.iHOffset), &(m_dsImg.iVOffset), &(m_dsImg.iWidth), &(m_dsImg.iHeight));		

		status = STATUS_NOT_SUPPORTED;					//��״̬Ϊ��Ч
	}
	else
	{
		printf("[MX500] init camera error\n");
		return -1;
	}

	// ͼ��ֱ��ת,��Ϊ�����ʽ��IplImage*˳��ͬ����Ҫ��ֱ��ת
	CameraSetMirror(m_nCameraID, MIRROR_DIRECTION_VERTICAL, TRUE);


	//�����
	status = CameraPlay(m_nCameraID);		// ������Ƶ��
	if (status == STATUS_OK)
	{
		printf("[MX500] playing...\n");
		status = STATUS_NOT_SUPPORTED;			//��״̬Ϊ��Ч
	}
	else
	{
		printf("[MX500]play camera error\n");
		return -1;
	}


	// ����֡��
	// FRAME_SPEED_NORMAL:����, FRAME_SPEED_HIGH:����, FRAME_SPEED_SUPER:������
	emDSFrameSpeed emFrameSpeed = FRAME_SPEED_SUPER;       
	CameraSetFrameSpeed(m_nCameraID, emFrameSpeed);

	//����һ���ڲ�����ͼ��
	_retireImage = cvCreateImage(cvSize(m_dsImg.iWidth, m_dsImg.iHeight), IPL_DEPTH_8U, 3);

	return 0;
}

/************************************************************************/
/* ���ܣ���ȡ��һ֡ͼ��                                                   */
/* ������ɹ�����0,������-1�����img                                    */
/************************************************************************/
int QueryFrameMX500(IplImage **img)
{
	BYTE* byImageBuf = NULL;
	emDSCameraStatus status = CameraGetImageBuffer(m_nCameraID, DATA_TYPE_RGB8, &byImageBuf);		// ��ȡRGB 24λͼ������
	if (status == STATUS_OK)						//����������ɣ���ʼ����IplImageͼ��
	{
		//תΪIplImage*ͼ��
		cvSetData(_retireImage, byImageBuf, m_dsImg.iWidth*3);
		if (NULL == *img)
		{
			*img = cvCreateImage(cvSize(m_dsImg.iWidth, m_dsImg.iHeight), IPL_DEPTH_8U, 3);
			cvCopy(_retireImage, *img, NULL);				//�������
		}
		else
		{
			cvResize(_retireImage, *img);					//�������
		}

		//�������
		CameraReleaseImageBuffer(m_nCameraID, DATA_TYPE_BGR8, byImageBuf);		// �ͷŻ���
		byImageBuf = NULL;
	}
	else
	{
		//printf("[MX500] read error\n");
		return -1;
	}

	return 0;
}

/************************************************************************************/
/* ���ܣ�����ʼ�����																	*/
/* ����ֵ���ɹ�����0��ʧ�ܷ���-1														*/
/************************************************************************************/
int UnConfigMX500()
{
	emDSCameraStatus status = CameraPlay(m_nCameraID);
	if (status == STATUS_OK)
	{
		CameraPowerDown(m_nCameraID);											//�رյ�Դ
		if (status == STATUS_OK)
		{
			cvReleaseImage(&_retireImage);											//�ͷŻ���ͼ��

			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		printf("[MX500] stop camera error\n");
		return -1;
	}
}