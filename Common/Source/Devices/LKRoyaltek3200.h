/*
 * Royaltek interface to Windows CE by Paolo
 */
#if defined(PNA) && defined(UNDER_CE)

extern bool DeviceIsRoyaltek3200;

extern bool Init_Royaltek3200(void);
extern void DeInit_Royaltek3200(void);

extern bool	Royaltek3200_ReadBarData();

extern int	Royaltek3200_GetAltitude(void);
extern float	Royaltek3200_GetPressure(void);
extern float	Royaltek3200_GetTemperature(void);


#endif // PNA only
