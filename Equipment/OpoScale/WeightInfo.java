//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by FernFlower decompiler)
//

package OnePlusOneAndroidSDK.ScalesOS;

import android.os.Parcel;
import android.os.Parcelable;

public class WeightInfo implements Parcelable {
    private String sMode;
    private String sStatus;
    private String sZero;
    private String sUnit;
    private String sNetWeight;
    private String sTareWeight;
    private String sGrossWeight;
    public static final Parcelable.Creator<WeightInfo> CREATOR = new Parcelable.Creator<WeightInfo>() {
        public WeightInfo createFromParcel(Parcel in) {
            return new WeightInfo(in);
        }

        public WeightInfo[] newArray(int size) {
            return new WeightInfo[size];
        }
    };

    public WeightInfo() {
    }

    public WeightInfo(String weight) {
        this.updata(weight);
    }

    protected WeightInfo(Parcel in) {
        this.sMode = in.readString();
        this.sStatus = in.readString();
        this.sZero = in.readString();
        this.sUnit = in.readString();
        this.sNetWeight = in.readString();
        this.sTareWeight = in.readString();
        this.sGrossWeight = in.readString();
    }

    public boolean updata(String weight) {
        try {
            if (weight != null && !weight.isEmpty()) {
                if (weight.length() > 100) {
                    return false;
                } else {
                    String[] split = weight.split(",");
                    if (split.length < 6) {
                        return false;
                    } else {
                        this.sMode = split[0];
                        this.sStatus = split[1];
                        this.sZero = split[2];
                        this.sUnit = split[3];
                        this.sNetWeight = split[4];
                        this.sTareWeight = split[5];
                        this.sGrossWeight = split[6];
                        return true;
                    }
                }
            } else {
                return false;
            }
        } catch (Exception var3) {
            return false;
        }
    }

    public String getMode() {
        switch (this.sMode) {
            case "N":
                return "Net";
            case "T":
                return "Tare";
            case "P":
                return "Tare";
            default:
                return "Error";
        }
    }

    public String getStatus() {
        switch (this.sStatus) {
            case "F":
                return "Flow";
            case "S":
                return "Stable";
            case "U":
                return "UnStable";
            default:
                return "Error";
        }
    }

    public boolean getZero() {
        return !this.sZero.equals("");
    }

    public String getUnit() {
        return this.sUnit;
    }

    public String getNetWeight() {
        return this.sNetWeight;
    }

    public String getTareWeight() {
        return this.sTareWeight;
    }

    public String getGrossWeight() {
        return this.sGrossWeight;
    }

    public String toString() {
        return "WeightInfo{sMode='" + this.sMode + '\'' + ", sStatus='" + this.sStatus + '\'' + ", sZero='" + this.sZero + '\'' + ", sUnit='" + this.sUnit + '\'' + ", sNetWeight='" + this.sNetWeight + '\'' + ", sTareWeight='" + this.sTareWeight + '\'' + ", sGrossWeight='" + this.sGrossWeight + '\'' + '}';
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.sMode);
        dest.writeString(this.sStatus);
        dest.writeString(this.sZero);
        dest.writeString(this.sUnit);
        dest.writeString(this.sNetWeight);
        dest.writeString(this.sTareWeight);
        dest.writeString(this.sGrossWeight);
    }
}
