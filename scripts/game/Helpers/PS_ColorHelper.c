class PS_ColorHelper
{
	static Color DesaturateColor(Color color, float factor)
	{
		float light = 0.3*color.R() + 0.6*color.G() + 0.1*color.B();
		return new Color(
			color.R() + factor * (light - color.R()),
			color.G() + factor * (light - color.G()),
			color.B() + factor * (light - color.B()),
			color.A()
		);
	}
	
	static Color ChangeLightColor(Color color, float factor)
	{
		return new Color(
			color.R() * factor,
			color.G() * factor,
			color.B() * factor,
			color.A()
		);
	}
}