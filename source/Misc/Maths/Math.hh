namespace Maths
{
	double PI = 3.14159265359;
	inline double ToRadians(double degree) noexcept
	{
		return (degree * (PI / 180)); 
	}
}