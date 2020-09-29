#include "FunModule.h"
#include <QtNetwork>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>

FunModule::FunModule() : Module("funutil", true), m_memeChannel(0)
{

	QObject::connect(&m_MemeManager, &QNetworkAccessManager::finished,
		this, [this](QNetworkReply* reply) {
			auto& client = UmikoBot::Instance();
			if (reply->error()) {
				qDebug() << reply->errorString();
				client.createMessage(m_memeChannel, reply->errorString());
			}

			QString in = reply->readAll();

			QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());
			auto obj = doc.object();
			bool isNsfw = obj["nsfw"].toBool();
			if (isNsfw) {
				m_MemeManager.get(QNetworkRequest(QUrl("https://meme-api.herokuapp.com/gimme")));
				return;
			}
			QString title = obj["title"].toString();
			QString url = obj["url"].toString();
			QString author = obj["author"].toString();
			QString postLink = obj["postLink"].toString();
			QString subreddit = obj["subreddit"].toString();

			Discord::Embed embed;
			Discord::EmbedImage img;
			img.setUrl(url);
			embed.setImage(img);
			embed.setTitle(title);
			Discord::EmbedFooter footer;
			footer.setText("Post was made by u/" + author + " on r/" + subreddit + ".\nSee the actual post here: " + postLink);
			embed.setFooter(footer);

			client.createMessage(m_memeChannel, embed);
		});

	RegisterCommand(Commands::FUN_MEME, "meme", [this](Discord::Client& client, const Discord::Message& message, const Discord::Channel& channel) 
		{

		QStringList args = message.content().split(' ');
		GuildSetting* setting = &GuildSettings::GetGuildSetting(channel.guildId());
		QString prefix = setting->prefix;


		if (args.first() != prefix + "meme")
			return;

		if (args.size() >= 2) {
			client.createMessage(message.channelId(), "**Wrong Usage of Command!** ");
			return;
		}

		m_memeChannel = channel.id();
		m_MemeManager.get(QNetworkRequest(QUrl("https://meme-api.herokuapp.com/gimme")));

		});

}