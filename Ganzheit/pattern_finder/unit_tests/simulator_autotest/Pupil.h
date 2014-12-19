#ifndef PUPIL_H
#define PUPIL_H


class Pupil {

	public:
		Pupil(int _radius, int _x = 0 , int _y = 0);

		void moveX(int inc_x);
		void moveY(int inc_y);
		void moveTo(int x, int y);

		void getPos(int &x, int &y) const;
		int getRadius() const {return radius;}

	private:
		int pos_x;
		int pos_y;
		int radius;

};

#endif

