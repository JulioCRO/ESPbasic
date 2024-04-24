class Fahrenheit
{
    public:
        Fahrenheit(){
        };

        
        float c2f(float Tc){
          float Tf;
          Tf = Tc * 9/5 + 32;
          return Tf;
        }

        float f2c(float Tf){
          float Tc;
          Tc = (Tf-32)*((float)5/9);
          return Tc;
        }
};