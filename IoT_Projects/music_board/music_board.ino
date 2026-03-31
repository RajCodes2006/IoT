#define BUZZER 25
#define CH 0
#define RES 8

#define BTN1 14
#define BTN2 27
#define BTN3 26

#define G4 392
#define A4 440
#define B4 494
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G5 784

enum SongType
{
	HAPPY_BDAY=1,
	TWINKLE_TWINKLE=2,
	JOHNNY_JOHNNY=3
};

int hb_melody[]={
	G4,G4,A4,G4,C5,B4,
	G4,G4,A4,G4,D5,C5,
	G4,G4,E5,C5,B4,A4,
	F5,F5,E5,C5,D5,C5
};

int hb_duration[]={
	250,250,500,500,500,1000,
	250,250,500,500,500,1000,
	250,250,500,500,500,1000,
	250,250,500,500,500,1000
};

int tt_melody[]={
	C5,C5,G4,G4,A4,A4,G4,
	F5,F5,E5,E5,D5,D5,C5
};

int tt_duration[]={
	400,400,400,400,400,400,800,
	400,400,400,400,400,400,800
};

int jj_melody[]={
	C5,D5,E5,C5,
	C5,D5,E5,C5,
	E5,F5,G5,
	E5,F5,G5
};

int jj_duration[]={
	300,300,600,600,
	300,300,600,600,
	400,400,800,
	400,400,800
};

void playSong(int melody[],int duration[],int len)
{
	for(int i=0;i<len;i++)
	{
		ledcWriteTone(CH,melody[i]);
		delay(duration[i]);
		ledcWriteTone(CH,0);
		delay(50);
	}
}

void playByType(SongType song)
{
	switch(song)
	{
		case HAPPY_BDAY:
			playSong(hb_melody,hb_duration,25);
			break;
		case TWINKLE_TWINKLE:
			playSong(tt_melody,tt_duration,14);
			break;
		case JOHNNY_JOHNNY:
			playSong(jj_melody,jj_duration,14);
			break;
	}
}

void setup()
{
	pinMode(BTN1,INPUT_PULLUP);
	pinMode(BTN2,INPUT_PULLUP);
	pinMode(BTN3,INPUT_PULLUP);

	ledcSetup(CH,2000,RES);
	ledcAttachPin(BUZZER,CH);
}

void loop()
{
	if(digitalRead(BTN1)==LOW)
	{
		playByType(HAPPY_BDAY);
		delay(300);
	}
	else if(digitalRead(BTN2)==LOW)
	{
		playByType(TWINKLE_TWINKLE);
		delay(300);
	}
	else if(digitalRead(BTN3)==LOW)
	{
		playByType(JOHNNY_JOHNNY);
		delay(300);
	}
}
