struct Vector
{
	float x, y, z;

	inline Vector()
	{
		Zero();
	}
	inline Vector(float flin)
	{
		x = flin;
		y = flin;
		z = flin;
	}
	
	inline Vector(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}
	
	inline void Zero()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	
	float& operator[](int a)
	{
		if (a == 0)
		{
			return x;
		}
			
		if (a == 1)
		{
			return y;
		}
			
		if (a == 2)
		{
			return z;
		}
		

	}
	
	
};

typedef Vector QAngle;