namespace Maths
{
    inline double ToRadians(double degree) noexcept
    {
        double pi = 3.14159265359; 
        return (degree * (pi / 180)); 
    }
}