/*
 * Copyright (C) 2015 Bilibili
 * Copyright (C) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package tv.danmaku.ijk.media.example.fragments;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import tv.danmaku.ijk.media.example.R;
import tv.danmaku.ijk.media.example.activities.VideoActivity;

public class SampleMediaListFragment extends Fragment {
    private ListView mFileListView;
    private SampleMediaAdapter mAdapter;

    public static SampleMediaListFragment newInstance() {
        SampleMediaListFragment f = new SampleMediaListFragment();
        return f;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        ViewGroup viewGroup = (ViewGroup) inflater.inflate(R.layout.fragment_file_list, container, false);
        mFileListView = (ListView) viewGroup.findViewById(R.id.file_list_view);
        return viewGroup;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        final Activity activity = getActivity();

        mAdapter = new SampleMediaAdapter(activity);
        mFileListView.setAdapter(mAdapter);
        mFileListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, final int position, final long id) {
                SampleMediaItem item = mAdapter.getItem(position);
                String name = item.mName;
                String url = item.mUrl;
                VideoActivity.intentTo(activity, url, name);
            }
        });

        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/8421be02-88a8-4a2f-8f26-25db1ff1785b.mp4","学画圆圈图");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/34d5b658-64a7-400b-b3bd-a167539a72ef.mp4","机械拼插00");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/c71fdcd3-d237-4145-b4e5-41e25292bfe3.mp4","机械拼插01");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/db317dbd-c677-4d7d-a3f9-5f29a3a4c2de.mp4","机械拼插02");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/871efea2-48c1-445f-8ee7-d42c3feda535.mp4","机械拼插03");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/60dc0646-0209-41ae-b4dd-2e421664cfda.mp4","机械拼插04");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/5baef45b-09e8-443a-b319-19a7023aee80.mp4","机械拼插05");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/1aa9303c-9997-497a-b9db-f54322e50ff9.mp4","机械拼插06");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/561769ae-1211-4846-bea1-d95122d81c81.mp4","机械拼插07");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/a368a69b-00d6-4e5d-af1b-d3990b9303d9.mp4","机械拼插08");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/b45f4b79-e28f-4c45-9756-ea413fc6093a.mp4","机械拼插09");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/a7a7e2e3-860f-4e61-a134-312bb22730f5.mp4","机械拼插10");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/mcourse/camp/video/1510f271-df99-4c73-b595-82d9e86daf22.mp4","机械拼插11");
        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/177fb6f7-1110-4ead-b371-c93c34b56d72.mp4","第0课：预备课");
        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/7f17e32a-9471-47be-83e4-3b7e560dc4c0.mp4","第1课：秋冬养阴，养阴护肾元气满满");
        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/28d56227-9c6b-4bd8-a189-cce627387d8b.mp4","第2课");
        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/4e6447ff-653f-4fda-8715-ab3290722615.mp4","第3课");
        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/849f2564-1e1f-48eb-bbf3-a887c3f334a3.mp4","第4课");

        //9.29开课数学
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/f16eb95f-73e7-44c3-8661-b287eee028c5.mp4","第0课：数与数感");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/f816ccc4-46df-491c-9361-0ddd4381a001.mp4","第1课：加法意义与应用");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/95e89cd2-02d4-4b0d-b173-149574a3c123.mp4","第2课：减法意义与应用");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/8a381b22-013e-417e-8d1a-0c2766158562.mp4","第3课：数轴加减法");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/13d711be-30d0-455a-9c7c-8ac1dbe9b323.mp4","亲子加油站");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/37022dbb-fe80-4a41-881a-81cd45e31521.mp4","第4课：凑十法");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/95d50d1a-5f6c-495b-92ce-1b80b45d73de.mp4","第5课：破十法");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/0da4e636-4859-4c21-bf01-ca29fe50a653.mp4","第6课：加减竖式与数字谜");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/ce8ed8b8-48fb-4974-86e3-4ee00b52ad83.mp4","第7课：减法竖式与数字谜");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/5d7d1e23-99d8-41a8-9019-4b438792916b.mp4","第8课：加减巧算1--凑整思想");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/4e5b6664-851d-4d36-9909-f42da092bc59.mp4","第9课：加减巧算2--基准数思想");
//
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/e96fb6a4-ab29-415e-9885-a026020ae516.mp4","第10课：乘法意义与应用");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/8fcd5c7f-9dbb-43c9-91d3-4aa829dc5c2a.mp4","第11课：加减巧算1--凑整思想");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/306b7c89-d984-4757-a573-8724e264bd6a.mp4","第12课：抵消大法的妙用");
//        mAdapter.addItem("https://cdn.kaishuhezi.com/kstory/microcourse/video/d28f82f1-c4d3-44df-b0f0-e005f5664a28.mp4","第13课：乘法巧算\"三律\"");



//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/bipbop_4x3_variant.m3u8", "bipbop basic master playlist");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear1/prog_index.m3u8", "bipbop basic 400x300 @ 232 kbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear2/prog_index.m3u8", "bipbop basic 640x480 @ 650 kbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear3/prog_index.m3u8", "bipbop basic 640x480 @ 1 Mbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear4/prog_index.m3u8", "bipbop basic 960x720 @ 2 Mbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear0/prog_index.m3u8", "bipbop basic 22.050Hz stereo @ 40 kbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/bipbop_16x9_variant.m3u8", "bipbop advanced master playlist");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear1/prog_index.m3u8", "bipbop advanced 416x234 @ 265 kbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear2/prog_index.m3u8", "bipbop advanced 640x360 @ 580 kbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear3/prog_index.m3u8", "bipbop advanced 960x540 @ 910 kbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear4/prog_index.m3u8", "bipbop advanced 1289x720 @ 1 Mbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear5/prog_index.m3u8", "bipbop advanced 1920x1080 @ 2 Mbps");
//        mAdapter.addItem("http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_16x9/gear0/prog_index.m3u8", "bipbop advanced 22.050Hz stereo @ 40 kbps");
    }

    final class SampleMediaItem {
        String mUrl;
        String mName;

        public SampleMediaItem(String url, String name) {
            mUrl = url;
            mName = name;
        }
    }

    final class SampleMediaAdapter extends ArrayAdapter<SampleMediaItem> {
        public SampleMediaAdapter(Context context) {
            super(context, android.R.layout.simple_list_item_2);
        }

        public void addItem(String url, String name) {
            add(new SampleMediaItem(url, name));
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                LayoutInflater inflater = LayoutInflater.from(parent.getContext());
                view = inflater.inflate(android.R.layout.simple_list_item_2, parent, false);
            }

            ViewHolder viewHolder = (ViewHolder) view.getTag();
            if (viewHolder == null) {
                viewHolder = new ViewHolder();
                viewHolder.mNameTextView = (TextView) view.findViewById(android.R.id.text1);
                viewHolder.mUrlTextView = (TextView) view.findViewById(android.R.id.text2);
            }

            SampleMediaItem item = getItem(position);
            viewHolder.mNameTextView.setText(item.mName);
            viewHolder.mUrlTextView.setText(item.mUrl);

            return view;
        }

        final class ViewHolder {
            public TextView mNameTextView;
            public TextView mUrlTextView;
        }
    }
}
