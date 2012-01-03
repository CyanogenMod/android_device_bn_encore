package com.cyanogenmod.stats;

import java.util.ArrayList;
import java.util.List;

import org.apache.http.NameValuePair;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.util.Log;

public class ReportingService extends Service {
    private static final String PREF_NAME = "CMStats";
    private static final String TAG = "CMStats";

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        if (isFirstBoot()) {
            promptUser();
            Log.d(TAG, "Prompting user for opt-in.");
        } else if (canReport() == true) {
            Log.d(TAG, "User has opted in -- reporting.");
            Thread thread = new Thread() {
                @Override
                public void run() {
                    report();
                }
            };
            thread.start();
        } else {
            Log.d(TAG, "User has not opted in -- skipping reporting.");
	    stopSelf();
        }
    }

    private boolean isFirstBoot() {
        SharedPreferences settings = getSharedPreferences(PREF_NAME, 0);
        boolean firstboot = settings.getBoolean("firstboot", true);
        return firstboot;
    }
    
    private void setCheckedIn(boolean checkedin) {
        SharedPreferences settings = getSharedPreferences(PREF_NAME, 0);
        SharedPreferences.Editor editor = settings.edit();
        
        editor.putBoolean("checkedin", checkedin);
        editor.commit();
        Log.d(TAG, "SERVICE: setting checkedin=" + checkedin);
    }

    private boolean canReport() {
        SharedPreferences settings = getSharedPreferences(PREF_NAME, 0);
        
        // Determine whether or not we have already checked in.
        boolean checkedin = settings.getBoolean("checkedin", false);
        
        // Determine opt-in status (opt-in by default)
        boolean optin = settings.getBoolean("optin", true);

        if (optin && !checkedin) {
            return true;
        } else {
            return false;
        }
    }

    private void report() {
        String deviceId = Utilities.getUniqueID(getApplicationContext());
        String deviceName = Utilities.getDevice();
        String deviceVersion = Utilities.getModVersion();
        String deviceCountry = Utilities.getCountryCode(getApplicationContext());
        String deviceCarrier = Utilities.getCarrier(getApplicationContext());
        String deviceCarrierId = Utilities.getCarrierId(getApplicationContext());

        Log.d(TAG, "SERVICE: Device ID=" + deviceId);
        Log.d(TAG, "SERVICE: Device Name=" + deviceName);
        Log.d(TAG, "SERVICE: Device Version=" + deviceVersion);
        Log.d(TAG, "SERVICE: Country=" + deviceCountry);
        Log.d(TAG, "SERVICE: Carrier=" + deviceCarrier);
        Log.d(TAG, "SERVICE: Carrier ID=" + deviceCarrierId);

        HttpClient httpclient = new DefaultHttpClient();
        HttpPost httppost = new HttpPost("http://stats.cyanogenmod.com/submit");
        try {
            List<NameValuePair> kv = new ArrayList<NameValuePair>(5);
            kv.add(new BasicNameValuePair("device_hash", deviceId));
            kv.add(new BasicNameValuePair("device_name", deviceName));
            kv.add(new BasicNameValuePair("device_version", deviceVersion));
            kv.add(new BasicNameValuePair("device_country", deviceCountry));
            kv.add(new BasicNameValuePair("device_carrier", deviceCarrier));
            kv.add(new BasicNameValuePair("device_carrier_id", deviceCarrierId));
            httppost.setEntity(new UrlEncodedFormEntity(kv));
            httpclient.execute(httppost);
            setCheckedIn(true);
        } catch (Exception e) {
            setCheckedIn(false);
            Log.e(TAG, "Got Exception", e);
        }

        stopSelf();
    }

    private void promptUser() {
        NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        Notification n = new Notification(R.drawable.icon,
                getString(R.string.notification_ticker), System.currentTimeMillis());
        Intent nI = new Intent(this, MainActivity.class);
        PendingIntent pI = PendingIntent.getActivity(getApplicationContext(), 0, nI, 0);
        n.setLatestEventInfo(getApplicationContext(),
                getString(R.string.notification_title),
                getString(R.string.notification_desc), pI);
        nm.notify(1, n);
    }
}
